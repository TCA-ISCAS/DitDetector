# -*- coding:utf-8 -*-
import os
import logging
import re

def traverse_files(dir_):
    """traverse the target dir, to get the file path list

    Args:
      dir_: target files' dir

    Return:
      file_path_list: list of file path
    """
    file_path_list = []
    for root, dirs, filename_list in os.walk(dir_):
        for file_name in filename_list:
            file_path = os.path.join(root, file_name)
            file_path_list.append(file_path)
    return file_path_list


def init_log(path):
    """init a log file to record info

    Args:
      path: path to record info

    Returns:
      logger: handle to log
    """
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    file_handler = logging.FileHandler(path, mode='a')
    file_handler.setLevel(logging.INFO)

    stream_handler = logging.StreamHandler()
    stream_handler.setLevel(logging.INFO)

    formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
    stream_handler.setFormatter(formatter)
    file_handler.setFormatter(formatter)

    logger.addHandler(file_handler)
    return logger



def delete_characters(doc):
    """
    去除特殊字符
    :param doc:
    :return:
    """
    doc = doc.split()

    doc = ' '.join(doc).replace(',', '').replace('.', '').replace('?', '').replace('!', '') \
        .replace('"', '').replace('@', '').replace(':', '').replace('(', '').replace(')', '') \
        .replace('-', '').replace('#', '').replace('/', '').replace('+', '').replace("'", '') \
        .replace('*', '').replace('|', '').replace('`', '').replace('[', '').replace(']', '') \
        .replace('{', '').replace('}', '').replace('&', '').replace('”', '').replace('‘', '') \
        .replace('’', '').replace('»', '').replace('®', '').replace('™', '').replace('«', '') \
        .replace('￥', '').replace('~', '').replace('\'', '').replace('°', '').replace('€', '') \
        .replace('>', '').replace('<', '').replace('_', '').replace('【', '').replace('】', '') \
        .replace('$', '') \
        .replace('\\', '').replace(';', '').replace('“', '').replace('...', '').replace('=', '')

    doc = re.sub('[0-9]', '', doc)
    doc = re.sub(r"\s+", " ", doc)
    return doc.strip()




