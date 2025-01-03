# -*- coding:utf-8 -*-
import os, sys
import pandas as pd
import torch

cur_dir = os.getcwd()
src_path = os.path.abspath(os.path.join(cur_dir, '..'))
sys.path.append(src_path)

from gensim import models
from gensim.models import Doc2Vec
from collections import defaultdict
from torchtext import data
from torchtext.vocab import Vectors
from DitDetector.basic_func import delete_characters


data_dir = os.path.join(cur_dir, '..', '..', 'data', 'ocr_res')
cur_data_dir = os.path.join(cur_dir, '..', '..', 'data', 'text_cnn_res')

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')


def generate_doc2vec_model(model_path, doc_list, label_list, vector_path):
    print('[Training doc2vec model...]')
    if os.path.exists(model_path):
        return Doc2Vec.load(model_path)
    docs = []
    for doc, label in zip(doc_list, label_list):
        tmp_doc = models.doc2vec.TaggedDocument(words=doc, tags=[label])
        docs.append(tmp_doc)

    doc2vec_model = Doc2Vec(docs, dm=0, vector_size=50, dbow_words=1, epochs=30)
    doc2vec_model.save(model_path)
    doc2vec_model.wv.save_word2vec_format(vector_path, binary=False)

    return doc2vec_model


def load_data():
    benign_path = os.path.join(data_dir, 'tesseract_benign.csv')
    df_benign = pd.read_csv(benign_path)
    b_label = [0] * df_benign.shape[0]
    df_benign['label'] = b_label

    malicious_path = os.path.join(data_dir, 'tesseract_malicious.csv')
    df_malicious = pd.read_csv(malicious_path)
    m_label = [1] * df_malicious.shape[0]
    df_malicious['label'] = m_label

    return df_benign, df_malicious


def delete_freq(texts):
    frequency = defaultdict(int)

    for text in texts:
        for token in text:
            frequency[token] += 1
    texts = [[token for token in text if frequency[token] > 1] for text in texts]  # 删除仅仅出现一次的词

    return texts


def clean_doc(df_data):
    docs = df_data['values'].to_list()

    res_doc = []
    for doc in docs[:]:
        if doc:
            doc = str(doc).lower()
            tmp_doc = delete_characters(doc)
            tmp_doc = list(tmp_doc.split())
            res_doc.append(tmp_doc)
        else:
            res_doc.append([''])

    res_doc = delete_freq(res_doc)
    res_doc = [' '.join(e) for e in res_doc]

    return res_doc


def correct_label(file_name):

    df_data = pd.read_csv(cur_data_dir + file_name)
    tmp_df = df_data[(df_data.isnull().T.any()) & df_data.label == 1]
    tmp_md5 = tmp_df.md5.to_list()
    df_data.loc[df_data.md5.isin(tmp_md5), 'label'] = 0
    new_path = os.path.join(cur_data_dir,  file_name.split('.')[0] + '_cl.csv')
    df_data.to_csv(new_path, index=False)


def generate_csv():
    train_csv_path = 'train_4_1.csv'
    test_csv_path = 'val.csv'

    if not (os.path.exists(train_csv_path) or not os.path.exists(test_csv_path)):

        print('preprocessing labels...')
        # load data and split the dataset
        df_benign, df_malicious = load_data()

        df_data = df_benign.append(df_malicious)
        label_list = df_data.label.to_list()
        md5_list = df_data.md5.to_list()
        doc_clr = clean_doc(df_data)
        df_datas = pd.DataFrame({'md5': md5_list, 'text': doc_clr, 'label': label_list})

        df_split = pd.read_csv(os.path.join(cur_data_dir, 'split_dataset.csv'))
        df_split.drop(['label'], axis=1)

        df_datas = df_datas.merge(df_split, on=['md5', 'label'], how='right')
        df_datas.to_csv(os.path.join(cur_data_dir, 'all_clr_data.csv'), index=False)

        #  train_1, train_2, val, test = 4:4:1:1
        df_train_1 = df_datas[df_datas.attr == 'train_4_1'].copy()
        df_train_2 = df_datas[df_datas.attr == 'train_4_2'].copy()
        df_val = df_datas[df_datas.attr == 'validation'].copy()
        df_test = df_datas[df_datas.attr == 'test'].copy()

        df_train = df_train_1.append(df_train_2)
        df_train.to_csv(os.path.join(cur_data_dir, 'train.csv'), index=False)

        df_train_1.to_csv(os.path.join(cur_data_dir, 'train_4_1.csv'), index=False)
        df_train_2.to_csv(os.path.join(cur_data_dir, 'train_4_2.csv'), index=False)
        df_val.to_csv(os.path.join(cur_data_dir, 'val.csv'), index=False)
        df_test.to_csv(os.path.join(cur_data_dir, 'test.csv'), index=False)

        correct_label('train.csv')
        correct_label('train_4_1.csv')
        correct_label('train_4_2.csv')
        correct_label('val.csv')
        correct_label('test.csv')


def preprocess_docs(train_name, val_name):
    """
    """
    generate_csv()

    fix_len = 1000
    #  initialize the data Field
    text = data.Field(sequential=True, lower=True, fix_length=fix_len)
    label = data.Field(sequential=False)
    fields = [('md5', None), ('text', text), ('label', label), ('attr', None)]

    train_4_vocab = None
    train_data_cl_path = os.path.join(cur_data_dir, 'train_cl.csv')
    if os.path.exists(train_data_cl_path):
        train_4_vocab = data.TabularDataset(
            path=train_data_cl_path,
            skip_header=True,
            format='csv',
            fields=fields
        )

    vector_path = os.path.join(cur_data_dir, 'doc2vec.vector')
    print(vector_path)
    model_path = os.path.join(cur_data_dir, 'doc2vec.model')

    if not os.path.exists(vector_path):
        my_vector_model = generate_doc2vec_model(model_path, train_4_vocab.text, train_4_vocab.label, vector_path)
        # my_vector_model.wv.save_word2vec_format(vector_path, binary=False)

    # build the vocabulary
    text.build_vocab(train_4_vocab, vectors=Vectors(name=vector_path))
    label.build_vocab(train_4_vocab)

    train, val = data.TabularDataset.splits(
        path=cur_data_dir,
        skip_header=True,
        train=train_name,
        validation=val_name,
        format='csv',
        fields=fields
    )

    vocab_size = len(text.vocab)
    embedding_dim = text.vocab.vectors.size()[-1]

    return train, val, vocab_size, embedding_dim


def split_dataset(train, val, batch_size, device):

    train_iter, val_iter = data.Iterator.splits((train, val),
                                                batch_sizes=(batch_size, batch_size),
                                                device=device,
                                                sort_key=lambda x:len(x.text),
                                                sort_within_batch=False,
                                                repeat=False)

    return train_iter, val_iter


def generate_doc_embedding(test_name):
    """
    """

    fix_len = 1000
    # fix_len = 100
    #  initialize the data Field
    text = data.Field(sequential=True, lower=True, fix_length=fix_len)
    label = data.Field(sequential=False)
    fields = [('md5', None), ('text', text), ('label', label), ('attr', None)]

    train_4_vocab = None
    train_data_cl_path = os.path.join(cur_data_dir, 'train_cl.csv')
    if os.path.exists(train_data_cl_path):
        train_4_vocab = data.TabularDataset(
            path=train_data_cl_path,
            skip_header=True,
            format='csv',
            fields=fields
        )

    vector_path = os.path.join(cur_data_dir, 'doc2vec.vector')
    # print(vector_path)
    model_path = os.path.join(cur_data_dir, 'doc2vec.model')

    if not os.path.exists(vector_path):
        my_vector_model = generate_doc2vec_model(model_path, train_4_vocab.text, train_4_vocab.label, vector_path)
        # my_vector_model.wv.save_word2vec_format(vector_path, binary=False)

    # build the vocabulary
    text.build_vocab(train_4_vocab, vectors=Vectors(name=vector_path))
    label.build_vocab(train_4_vocab)

    test_fields = [('filename', None), ('text', text)]
    test_path = os.path.join(cur_data_dir, test_name)
    test = data.TabularDataset(
        path=test_path,
        skip_header=True,
        format='csv',
        fields=test_fields
    )

    return test

