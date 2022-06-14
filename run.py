import requests
import json
import logging
import re
from datetime import datetime
from typing import cast
from uuid import uuid4

from threading import Thread
import os
import sys

from primitives.messages import CancelMessage, SetProjectMessage, SetSprintMessage, SetColumnMessage, SetAssigneeMessage, SetIssueMessage

from telegram import User, MessageEntity, Chat, InlineQueryResultArticle, InputTextMessageContent
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup, Message, ReplyKeyboardRemove, ReplyKeyboardMarkup
from telegram import ParseMode
from telegram.ext import (
    Updater,
    Filters,
    CallbackContext,
    MessageHandler,
    CommandHandler,
    CallbackQueryHandler,
    PicklePersistence,
    InlineQueryHandler,
    ConversationHandler
)

logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

logger = logging.getLogger(__name__)

YT = 'https://YT'

def start(update: Update, context: CallbackContext) -> None:
    user = update.message.from_user
    if user.username not in context.bot_data['users']:
        context.bot_data['users'][user.username] = user.id
    update.message.reply_text(' С помощью этого бота вы можете:\nЗадать/удалить токен\nСоздавать задачу на YouTrack в инлайн режиме\n')


def help(update: Update, context: CallbackContext) -> None:
    update.message.reply_text(f'<b>Создать токен:</b> наберите /create_token и следуйте инструкции\n\n<b>Поменять токен:</b> отправьте боту в сообщении свой новый токен\n\n<b>Cоздать задачу:</b> наберите @cds_ytbot и начните писать описание задачи. Затем нажмите на всплывающее название задачи и нажмите на кнопку <b>Создать</b>', parse_mode=ParseMode.HTML)


def fill_headers(update: Update, context: CallbackContext, method='get'):
    try:
        username = update.message.from_user.username
    except AttributeError:
        username = context.user_data['username']
    if username not in context.bot_data:
        logger.info(f'{username} doesn\'t have token')
        return

    token = context.bot_data[username]
    headers = {'Accept': 'application/json', 'Cache-Control': 'no-cache', 'Authorization': f'Bearer {token}'}
    if method == 'post':
        headers['Content-Type'] = 'application/json'

    return headers


def ok(req, update: Update, context: CallbackContext) -> bool:
    if req.status_code == requests.codes.ok:
        return True
    else:
        logger.info(req.text)
        logger.info(req.json()['error_description'])
        logger.info(req.raise_for_status())
        return False


def make_keyboard(pairs: list, extra_buttons=0, width=3) -> list:
    keyboard = []
    row = []
    count = 1
    size = len(pairs) - extra_buttons

    for i in range(size):
        if count > width:
            keyboard.append(row)
            row = []
            count = 1
        row.append(InlineKeyboardButton(text=pairs[i][0], callback_data=pairs[i][1]))
        count += 1
    keyboard.append(row)

    row = []
    for i in range(extra_buttons):
        row.append(InlineKeyboardButton(text=pairs[size+i][0], callback_data=pairs[size+i][1]))
    keyboard.append(row)

    return keyboard


def create_issue(issue: SetIssueMessage) -> dict:
    payload = {
                'customFields': [
                                    {'value': {'name': issue.column_name, '$type': 'StateBundleElement'}, 'name': 'State', '$type': 'StateIssueCustomField'}, # Column
                                    {'value': [{'login': issue.assignee_login, '$type': 'User'}], 'name': 'Assignee', '$type': 'MultiUserIssueCustomField'}, # Assignee
                                    {'value': [{'name': issue.sprint_name, '$type': 'VersionBundleElement'}], 'name': 'Sprints', '$type': 'MultiVersionIssueCustomField'} # Sprints
                                ],
                    'summary': issue.summary,
                    'description': issue.description,
                    'project': {'id': issue.project_id}
              }
    return payload


def create_token(update: Update, context: CallbackContext) -> None:
    username = update.message.from_user.username
    context.user_data['username'] = username
    old_token = context.bot_data.get(username)

    if old_token:
        update.message.reply_text('У вас уже есть токен')
        return

    update.message.reply_text('Сейчас вы должны создать токен, скопировать и отправить боту в сообщении. После ввода логина и пароля, нажмите на фото профиля -> Профиль -> Обновление анкетных данных и управление логинами -> Аутентификация -> Новый токен', reply_markup=InlineKeyboardMarkup([[InlineKeyboardButton(text='Создать токен', url='http://37.18.9.12')]]))


def save_token(update: Update, context: CallbackContext):
    token = update.message.text
    context.bot.delete_message(update.effective_chat.id, update.message.message_id)

    username = update.message.from_user.username
    headers = {'Accept': 'application/json', 'Cache-Control': 'no-cache', 'Authorization': f'Bearer {token}'}
    params = {'fields': 'id,name'}
    req = requests.get(YT+'/api/admin/projects', params=params, headers=headers)

    if req.status_code == requests.codes.ok:
        logger.info(f'@{username} has set token')
        context.bot_data[username] = token
        update.message.reply_text('Мы сохранили ваш токен!')
    elif req.status_code == 401:
        update.message.reply_text('Токен не подходит')
    else:
        update.message.reply_text(f'Похоже что-то пошло не так. Токен не сохранился')
        logger.info(req.raise_for_status())


def delete_token(update: Update, context: CallbackContext):
    username = update.message.from_user.username

    if username in context.bot_data:
        del context.bot_data[username]
        update.message.reply_text('Мы удалили ваш токен')
        logger.info(f'@{username} has deleted his token')
    else:
        update.message.reply_text('У вас нет токена')


def projects_name_id(update: Update, context: CallbackContext, Project: SetProjectMessage):
    params = {'fields': 'id,name'}
    headers = fill_headers(update, context)
    req = requests.get(YT+'/api/admin/projects', params=params, headers=headers)
    if not ok(req, update, context):
        return None

    projects = req.json()
    names_ids = [(project['name'], SetSprintMessage(project['name'], project['id'],
                                                    Project.summary, Project.description)) for project in projects]
    names_ids.append(('Отмена', CancelMessage()))

    return names_ids


def sprints_name(update: Update, context: CallbackContext, Sprint: SetSprintMessage):
    headers = fill_headers(update, context)
    params = {'fields': 'id,name'}
    req = requests.get(YT+'/api/agiles/', params=params, headers=headers)
    if not ok(req, update, context):
        return

    agiles = req.json()
    for agile in agiles:
        if agile['name'] == Sprint.project_name:
            agile_id = agile['id']
    params = {'fields': 'id,name'}
    req = requests.get(YT+'/api/agiles/'+agile_id+'/sprints', params=params, headers=headers)
    if not ok(req, update, context):
        return

    sprints = req.json()
    names = [(sprint['name'], SetColumnMessage(Sprint.project_name, Sprint.project_id,
                                               sprint['name'], sprint['id'],
                                               Sprint.summary, Sprint.description)) for sprint in sprints]
    names.append(('Назад', Sprint.to_SetProjectMessage()))
    names.append(('Отмена', CancelMessage()))

    return names


def project_columns(update: Update, context: CallbackContext, Column: SetColumnMessage):
    headers = fill_headers(update, context)
    params = {'fields': 'name,id'}
    req = requests.get(YT+'/api/agiles/', params=params, headers=headers)
    if not ok(req, update, context):
        return

    agiles = req.json()
    for agile in agiles:
        if agile['name'] == Column.project_name:
            agile_id = agile['id']

    params = {'fields': 'columnSettings(columns(presentation,id))'}
    req = requests.get(YT+'/api/agiles/'+agile_id, params=params, headers=headers)
    if not ok(req, update, context):
        return

    columns = req.json()['columnSettings']['columns']
    columns = [(column['presentation'], SetAssigneeMessage(Column.project_name, Column.project_id,
                                                        Column.sprint_name, Column.sprint_id,
                                                        column['presentation'], column['id'],
                                                        Column.summary, Column.description)) for column in columns]
    columns.append(('Назад', Column.to_SetSprintMessage()))
    columns.append(('Отмена', CancelMessage()))

    return columns


def project_users(update: Update, context: CallbackContext, Assignee: SetAssigneeMessage):
    headers = fill_headers(update, context)
    params = {'fields': 'team(id,users(login,name)),id,name', 'query': 'name: ' + Assignee.project_name}
    req = requests.get(YT+'/hub/api/rest/projects/', params=params, headers=headers)
    if not ok(req, update, context):
        return

    project = req.json()['projects'][0]
    users = project['team']['users']
    users = [(user['name'], SetIssueMessage(Assignee.project_name, Assignee.project_id,
                                            Assignee.sprint_name, Assignee.sprint_id,
                                            Assignee.column_name, Assignee.column_id,
                                            Assignee.summary, Assignee.description,
                                            user['name'], user['login'])) for user in users]
    users.append(('Назад', Assignee.to_SetColumnMessage()))
    users.append(('Отмена', CancelMessage()))

    return users


def cancel(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    query.edit_message_text('Создание отменено')
    # query.delete_message() # не работает. В группах бот не может удалять сообщения с кнопками


def post_issue_inline(update: Update, context: CallbackContext) -> None:
    query = update.inline_query.query
    if query != '':
        if has_token(update, context):
            if '.' in query:
                summary = query[:query.find('.')].strip()
                description = query[query.find('.')+1:].strip()
            else:
                summary = query
                description = ''

            project = SetProjectMessage(summary, description)
            names_ids = projects_name_id(update, context, project)
            article = InlineQueryResultArticle(
                id=str(uuid4()),
                title=query,
                input_message_content=InputTextMessageContent(f'Название: {summary}\nОписание: {description}\n\nВыберите проект'),
                reply_markup=InlineKeyboardMarkup(make_keyboard(names_ids, extra_buttons=1))
            )
            update.inline_query.answer([article])


def set_project(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    project = cast(SetProjectMessage, query.data)
    names_ids = projects_name_id(update, context, project)
    reply_markup = InlineKeyboardMarkup(make_keyboard(names_ids, extra_buttons=1))
    query.edit_message_text(f'Название: {project.summary}\nОписание: {project.description}\n\nВыберите проект', reply_markup=reply_markup)


def set_sprint(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    sprint = cast(SetSprintMessage, query.data)
    names = sprints_name(update, context, sprint)
    reply_markup = InlineKeyboardMarkup(make_keyboard(names, extra_buttons=2))
    query.edit_message_text(f'Название: {sprint.summary}\nОписание: {sprint.description}\n\nВыберите спринт', reply_markup=reply_markup)


def set_column(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    column = cast(SetColumnMessage, query.data)
    columns = project_columns(update, context, column)
    reply_markup = InlineKeyboardMarkup(make_keyboard(columns, extra_buttons=2))
    query.edit_message_text(f'Название: {column.summary}\nОписание: {column.description}\n\nВыберите столбец', reply_markup=reply_markup)


def set_assignee(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    assignee = cast(SetAssigneeMessage, query.data)
    users = project_users(update, context, assignee)
    reply_markup = InlineKeyboardMarkup(make_keyboard(users, extra_buttons=2, width=2))
    query.edit_message_text(f'Название: {assignee.summary}\nОписание: {assignee.description}\n\nКому назначить', reply_markup=reply_markup)


def set_issue(update: Update, context: CallbackContext) -> None:
    query = update.callback_query
    issue = cast(SetIssueMessage, query.data)
    headers = fill_headers(update, context, 'post')
    params = {'fields': 'id,name'}
    payload = create_issue(issue)

    req = requests.post(YT+'/api/issues', params=params, headers=headers, data=json.dumps(payload))
    if not ok(req, update, context):
        return
    query.edit_message_text(f'Название: {issue.summary}\nОписание: {issue.description}\nЗадача поставлена в\n<b>{issue.project_name} \U00002911 {issue.sprint_name} \U00002911 {issue.column_name}</b>', parse_mode=ParseMode.HTML)


def has_token(update: Update, context: CallbackContext) -> bool:
    try:
        user = update.inline_query.from_user
    except:
        user = update.message.from_user

    name = user.first_name + ' ' + user.last_name
    if user.username in context.bot_data:
        return True

    try:
        update.message.reply_text(f'У {name} нет токена')
    except AttributeError:
        article = InlineQueryResultArticle(
            id=str(uuid4()),
            title='У вас нет токена',
            input_message_content=InputTextMessageContent('Для создания токена нажмите команду /create_token')
        )
        update.inline_query.answer([article])

    return False


def error_handler(update: object, context: CallbackContext) -> None:
    logger.error(msg="Exception while handling an update:", exc_info=context.error)


def main():
    bot_token = 'secret_token'
    persistence = PicklePersistence(filename='helperbot.pkl')
    updater = Updater(bot_token, arbitrary_callback_data=True, persistence=persistence, use_context=True)

    dispatcher = updater.dispatcher

    dispatcher.add_handler(CommandHandler('start', start))
    dispatcher.add_handler(CommandHandler('help', help))
    dispatcher.add_handler(CommandHandler('create_token', create_token))
    dispatcher.add_handler(CommandHandler('delete', delete_token))

    dispatcher.add_handler(MessageHandler(Filters.regex(r'^perm:'), save_token))

    dispatcher.add_handler(InlineQueryHandler(post_issue_inline))

    dispatcher.add_handler(CallbackQueryHandler(cancel, pattern=CancelMessage))
    dispatcher.add_handler(CallbackQueryHandler(set_project, pattern=SetProjectMessage))
    dispatcher.add_handler(CallbackQueryHandler(set_sprint, pattern=SetSprintMessage))
    dispatcher.add_handler(CallbackQueryHandler(set_column, pattern=SetColumnMessage))
    dispatcher.add_handler(CallbackQueryHandler(set_assignee, pattern=SetAssigneeMessage))
    dispatcher.add_handler(CallbackQueryHandler(set_issue, pattern=SetIssueMessage))

    dispatcher.add_error_handler(error_handler)

    updater.start_polling()
    updater.idle()

if __name__ == "__main__":
    main()
