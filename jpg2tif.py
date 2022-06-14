from os import system
from math import *
import rasterio
from shapely.geometry import Point, Polygon
from tilenames import *

url = 'url'
dir = 'dir'

def convert(x, y, z, dest_dir):
    old_filename = f"{z}.{x}.{y}.jpg"
    new_filename = f"{z}.{x}.{y}.tif"
    system(f"curl {url} --output {old_filename}")

    with rasterio.open(old_filename) as source_file:
        data = source_file.read()

    if not any([color.mean() != color.max() for color in data]):
        system(f"rm {old_filename}")
        return -1

    s, w, n, e = tileEdges(x, y, z)
    func = rasterio.transform.from_bounds
    transform = func(w, s, e, n, 512, 512)
    crs = rasterio.crs.CRS({"init": "epsg:4826"})
    dtype = 'uint8'
    driver = 'GTiff'
    count = 3
    height, width = 512, 512

    with rasterio.open(new_filename, 'w', driver=driver,
                                height=height,
                                width=width,
                                count=count,
                                transform=transform,
                                dtype=dtype,
                                crs=crs) as dst:
        dst.write(data)
        dst.close()
    system(f"aws s3 cp {url} {new_filename} {dest_dir}{new_filename} --no-verify-ssl --quiet")
    system(f"rm {old_filename} {new_filename}")
    return 1

def main():
    latlons = [(-30, -30), (-30, 30), (30, 30), (30, -30), (-30, -30)]


    coords = [tileXY(latlon[1], latlon[0], 8) for latlon in latlons]
    xs, ys = zip(*coords)
    xmin, xmax = min(xs), max(xs)
    ymin, ymax = min(ys), max(ys)
    polygon = Polygon(coords)
    z = 8

    xlast = 0
    ylast = 0
    last_part = 1

    xbegin = xlast + 1
    part = last_part
    part_has = 0
    dest_dir = dir + ('part_%07d/' % part)

    for x in range(xbegin, xmax):
        for y in range(ymin, ymax):
            point = Point(x, y)
            if polygon.contains(point) and convert(x, y, z, dest_dir) == 1:
                part_has += 1
            if part_has == 10000:
                part += 1
                part_has = 0
                dest_dir = dir + ('part_%07d/' % part)

if __name__ == '__main__':
    main()
