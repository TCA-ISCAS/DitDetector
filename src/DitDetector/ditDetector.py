# -*- coding:utf-8 -*-
import os
import shutil
import sys
import pandas as pd
import pytesseract
import time
from PIL import Image
import argparse

cur_dir = os.getcwd()
src_path = os.path.abspath(os.path.join(cur_dir, '..'))

sys.path.append(os.path.abspath(src_path))
sys.path.append(os.path.join(src_path, 'TextCNN'))
sys.path.append(os.path.join(src_path, 'MobileNetV3'))
sys.path.append(os.path.join(src_path, 'BiModal'))

from text_preprocess_doc import generate_doc_embedding
from textcnn_infer import text_encoder
from mobilenet_infer import get_visual_fea_representation
from bimodal_infer import bimodal_detector
from basic_func import traverse_files, delete_characters


data_dir = os.path.join(cur_dir, '..', '..', 'data')


def tesseract_ocr(img_path_list, file_name_list, text_path):
    """
    Args:
        img_path_list: image paths to extract text
        file_name_list: image file name
        text_path: path to save text content into a '.csv' file

    Returns:
        None
    """
    if os.path.exists(text_path):
        df_data = pd.read_csv(text_path)
        data_filename_list = df_data.file_name.to_list()

        if data_filename_list == file_name_list:
            print('have extract the text')
            return

    print('[Extracting the text from images...]')
    text_list = []
    count = 0
    for img_path in img_path_list:
        if os.path.exists(img_path):
            text = ''
            image = Image.open(img_path)
            for frame in range(min(3, image.n_frames)):
                cur_img = image.seek(frame)
                tmp_img = image.tell()
                filename = str(tmp_img) + "_{}.png".format(int(time.time()))
                image.save(filename)
                text += pytesseract.image_to_string(Image.open(filename)) + ' '
                os.remove(filename)

            text = delete_characters(text)
            text_list.append(text)

            count += 1
            if (count % 20 == 0) or (count == len(img_path_list)):
                print('process %d files' % count)

    df_data = pd.DataFrame({'file_name': file_name_list, 'values': text_list})
    df_data.to_csv(text_path, index=False)

    print('[Done]')
    return


def get_text_fea_representation(doc_embeddings):
    """
    Args:
        doc_embeddings: input data to textual encoder as a tensor

    Returns:
        text_representation: textual feature representations

    """
    print('[Generating the text feature representation...]')
    textCNN = text_encoder()
    text_representation = textCNN.get_fea_representation(doc_embeddings)
    print('[Done]')
    return text_representation


def export_img_from_files(files_path_list, exp_img_path_list):
    oit_dir = os.path.join(cur_dir, '..', 'oit_linux_8.5.4', 'sdk', 'demo')

    cmd_line_prefix = 'cd ' + oit_dir + ' && ./exsimple '
    for tmp_path, exp_img_path in zip(files_path_list, exp_img_path_list):
        cmd_line = cmd_line_prefix + tmp_path + ' ' + exp_img_path
        os.system(cmd_line)

    return None


def get_args():
    parser = argparse.ArgumentParser(description='DitDetector - detect malicious Microsoft Files based on deceptive information')

    parser.add_argument('-m', '--ms_files', default='ms_files', type=str, required=False, help='dir to microsoft files to detect')
    parser.add_argument('-e', '--export_img', default='export_img', type=str, required=False, help='dir to store the export images by OIT')

    args = parser.parse_args()

    return args


def main(args):

    tmp_dir = os.path.join(data_dir, 'test_data')

    ms_files = os.path.join(tmp_dir, args.ms_files)
    exp_img_dir = os.path.join(tmp_dir, args.export_img)
    shutil.rmtree(exp_img_dir, ignore_errors=True)
    os.makedirs(exp_img_dir)
    text_path = os.path.join(tmp_dir, 'docText.csv')

    """
        # export image via Oracle OIT
    """
    time_start = time.time()
    files_path_list = traverse_files(ms_files)
    file_name_list = [os.path.basename(e).split('.')[0] + '.tiff' for e in files_path_list]
    exp_img_path_list = [os.path.join(exp_img_dir, e) for e in file_name_list]
    export_img_from_files(files_path_list, exp_img_path_list)
    tt1 = time.time()

    """
        # extract strings via OCR Tesseract
    """
    tesseract_ocr(exp_img_path_list, file_name_list, text_path)
    tt2 = time.time()

    """
        # encode visual information via MobileNet
    """
    visual_representation = get_visual_fea_representation(exp_img_path_list)
    tt3 = time.time()

    """
        # generate doc embedding via doc2vec
        # encode textual information via TextCNN
    """
    doc_embeddings = generate_doc_embedding(text_path)
    text_representations = get_text_fea_representation(doc_embeddings)
    tt4 = time.time()

    """
        # predict deceptive information via Bimodal
    """
    preds = bimodal_detector(visual_representation, text_representations)

    time_end = time.time()

    df_res = pd.DataFrame({'file_name': file_name_list, 'detection': preds})
    print('\n[Results.]\n', df_res)
    print('=== Total time cost ===\t{:.6f}s'.format(time_end - time_start))
    print('[OIT time cost]\t        {:.6f}s'.format(tt1-time_start))
    print('[OCR time cost]\t        {:.6f}s'.format(tt2-tt1))
    print('[MobileNetV3 time cost]\t{:.6f}s'.format(tt3-tt2))
    print('[TextCNN time cost]\t{:.6f}s'.format(tt4-tt3))
    print('[Bimodal time cost]\t{:.6f}s'.format(time_end-tt4))


if __name__ == "__main__":
    args = get_args()
    main(args)
