import argparse
import configparser
import os
import pickle
import signal
import sys
from functools import partial
from multiprocessing import Pool
from shapely.geometry import box
from rasterio import warp

import boto3
import rasterio
from shapely import geometry
from shapely.wkt import loads
from tqdm import tqdm

from osm.OSM import OSMClient, Connection
from osm.utils import clip
from osm.utils import from_name

def signal_handler(sig, frame, pool=None):
    if pool is not None:
        pool.terminate()
    sys.exit(0)

def get_s3_client(config):
    session = boto3.session.Session()

    s3_client = session.client(
        service_name='s3',
        aws_access_key_id=config["s3"]["aws_access_key_id"],
        aws_secret_access_key=config["s3"]["aws_secret_access_key"],
        endpoint_url=config["s3"]["endpoint_url"]
    )

    return s3_client


def get_bucket_name(client):

    return client.list_buckets()['Buckets'][0]['Name']


def get_all_objects(dfs, transform, crs, osm_crs):
    
    dict_ = {}

    for _, row in dfs['polygon'].iterrows():        
        osm_id, result = describe_row(row, transform=transform, crs=crs, osm_crs=osm_crs)
        if osm_id is not None:
            dict_[osm_id] = result
        else:
            pass

    for _, row in dfs['line'].iterrows():       
        osm_id, result = describe_row(row, transform=transform, crs=crs, osm_crs=osm_crs)
        if osm_id is not None:
            dict_[osm_id] = result
        else:
            pass
        
    return dict_


def describe_row(row, transform, crs, osm_crs, max_x=512, max_y=512):
    result = {}
    osm_id = None

    for key, value in row[~row.isna()].items():
        if key == "osm_id":
            osm_id = value

        elif key in ['z_order', 'way', 'way_area']:
            continue
        
        elif key == "coords":
            geometry = loads(row.coords)
            geometry_type = geometry.type

            geom = None
            if geometry_type == "Polygon":
                geom = to_pixels_for_polygon(transform, crs, osm_crs, geometry)
                
            elif geometry_type == "MultiPolygon":
                geom = to_pixels_for_multipolygon(transform, crs, osm_crs, geometry)
                
            elif geometry_type == "LineString":
                geom = to_pixels_for_line(transform, crs, osm_crs, geometry)

            if geom is not None:
                geom = clip(geom, max_x, max_y)

                if not geom.is_empty:
                    result[key] = geom.wkt
                else:
                    return None, None
            else:
                return None, None
            
        elif key == "tags":
            result[key] = str([key.replace(":", "_")+":"+value.replace(":", "_") for key, value in row['tags'].items()])

        else:
            result[key] = value
            
    return osm_id, result


def to_pixels_for_polygon(master_transform,
                          master_crs,
                          geom_crs,
                          geom):
    
    geom_projected = geometry.shape(warp.transform_geom(geom_crs, master_crs, geom))
    exterior = geom_projected.exterior
    interiors = geom_projected.interiors

    cols, rows = rasterio.transform.rowcol(master_transform, *zip(*exterior.coords))
    exterior_coords = list(zip(rows, cols))
    
    interiors_coors = []
    for interior in interiors:
        cols, rows = rasterio.transform.rowcol(master_transform, *zip(*interior.coords))
        interior_coords = list(zip(rows, cols))
        interiors_coors.append(interior_coords)

    poly = geometry.Polygon(exterior_coords, interiors_coors)
    
    return poly


def to_pixels_for_multipolygon(master_transform,
                               master_crs,
                               geom_crs,
                               geom):
    poly = geometry.MultiPolygon([
        to_pixels_for_polygon(master_transform, master_crs, geom_crs, g) for g in geom.geoms])

    return poly


def to_pixels_for_line(master_transform,
                       master_crs,
                       geom_crs,
                       geom):
    
    geom_projected = geometry.shape(warp.transform_geom(geom_crs, master_crs, geom))
    
    cols, rows = rasterio.transform.rowcol(master_transform, *zip(*geom_projected.coords))
    coords = list(zip(rows, cols))
    
    line = geometry.LineString(coords)
    
    return line


def load_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--in_dir', help='Input directory path', type=str, required=True)
    parser.add_argument('--out_dir', help='Output directory path', type=str, required=True)
    parser.add_argument('--workers', help='Number of workers', type=int, required=True)
    parser.add_argument('--batch_from', help='Batch number to start with', type=int, required=True)
    parser.add_argument('--batch_to', help='Batch number to end with', type=int, required=True)
        
    return parser.parse_args()


def get_filenames(s3_client, bucket_name, tile_prefix):
    all_objects2 = []
    paginator = s3_client.get_paginator('list_objects_v2')
    pages = paginator.paginate(Bucket=bucket_name, Prefix=tile_prefix)
    for page in pages:
        all_objects2 += page['Contents']
    return all_objects2


def get_target(file_name, osm_client, osm_crs=4326):
    name = os.path.basename(file_name).replace('.tif', '') # name = 17.x.y
    bounds, transform, crs = from_name(name)
    bounds = geometry.shape(warp.transform_geom(crs, osm_crs, box(*bounds)))

    geometries = ['polygon', 'line']
    dfs = {}
    for g in geometries:
        dfs[g] = osm_client.query(wkt=bounds.wkt, geom_type=g, crs=osm_crs)

    tile = {}
    tile['name'] = file_name
    tile['coordinates'] = bounds.wkt
   
    objects = get_all_objects(dfs=dfs, transform=transform, crs=crs, osm_crs=osm_crs)
    tile['objects'] = objects
    
    return tile


def routine(path_tile, output_path, config, prefix):
    endpoint = config["s3"]["endpoint_url"]

    connection = Connection(config["connection"]["host"],
                            config["connection"]["port"],
                            config["connection"]["database"],
                            config["connection"]["user"])

    osm_client = OSMClient(connection)

    s3_client = get_s3_client(config)
    bucket_name = get_bucket_name(s3_client)
    
    tile_prefix = os.path.join(prefix, path_tile)
    all_objects2 = get_filenames(s3_client, bucket_name, tile_prefix)
    list_of_minitiles = [name['Key'] for name in all_objects2]

    for path in list_of_minitiles:
        basename = os.path.basename(path)               # z.x.y.tif
        result = get_target(path, osm_client)

        dir = os.path.join(output_path, path_tile)
        s3_name = os.path.join(dir, basename)
        local_name = path_tile[:-1] + '_' + basename

        with open(local_name, 'wb') as f:
            pickle.dump(result, f, protocol=pickle.HIGHEST_PROTOCOL)

        with open(local_name, 'rb') as f:
            s3_client.upload_fileobj(f, bucket_name, s3_name)
   
        os.system(f"rm {local_name}")


def main():
    args = load_args()
    
    input_path = args.in_dir
    output_path = args.out_dir
    num_workers = args.workers

    batch_from = args.batch_from
    batch_to = args.batch_to
    
    config = configparser.ConfigParser()
    config.read("resources/config.ini")

    list_of_part = ['part_%09d/' % b for b in range(batch_from, batch_to + 1)]

    with Pool(num_workers) as p:
        partial_signal_handler = partial(signal_handler, pool = p)
        signal.signal(signal.SIGINT, partial_signal_handler)
        list(tqdm(p.imap_unordered(partial(routine,
                                           output_path=output_path,
                                           config=config,
                                           prefix=input_path), list_of_part), total=len(list_of_part)))

#   One thread: replace 'with Pool...' with 'for part...'
#   for part in list_of_part:
#       routine(part, output_path, config, input_path)

if __name__ == "__main__":
    main()
