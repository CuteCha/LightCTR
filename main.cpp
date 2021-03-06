//
//  main.cpp
//  LightCTR
//
//  Created by SongKuangshi on 2017/9/23.
//  Copyright © 2017年 SongKuangshi. All rights reserved.
//

#include <iostream>
#include "LightCTR/common/time.h"
#include "LightCTR/common/system.h"

#include "LightCTR/fm_algo_abst.h"
#include "LightCTR/train/train_fm_algo.h"
#include "LightCTR/predict/fm_predict.h"

#include "LightCTR/gbm_algo_abst.h"
#include "LightCTR/train/train_gbm_algo.h"
#include "LightCTR/predict/gbm_predict.h"

#include "LightCTR/em_algo_abst.h"
#include "LightCTR/train/train_gmm_algo.h"
#include "LightCTR/train/train_tm_algo.h"

#include "LightCTR/train/train_embed_algo.h"

#include "LightCTR/dl_algo_abst.h"
#include "LightCTR/train/train_cnn_algo.h"
#include "LightCTR/train/train_rnn_algo.h"

#include "LightCTR/train/train_vae_algo.h"

#include "LightCTR/train/train_nfm_algo.h"
using namespace std;

// Attention to check config in GradientUpdater
#define TEST_NFM

/* Recommend Configuration
 * FM/NFM batch=50 lr=0.1
 * VAE batch=1 lr=0.2
 * CNN batch=10 lr=0.03
 * RNN batch=10 lr=0.03
 */

size_t GradientUpdater::__global_minibatch_size(50);
double GradientUpdater::__global_learning_rate(0.1);
double GradientUpdater::__global_sparse_rate(0.4);
double GradientUpdater::__global_lambdaL2(0.001f);
double GradientUpdater::__global_lambdaL1(1e-5);

bool GradientUpdater::__global_bTraining(true);

int main(int argc, const char * argv[]) {
    int T = 200;
    
#ifdef TEST_FM
    FM_Algo_Abst *train = new Train_FM_Algo(
                        "./data/train_sparse.csv",
                        /*epoch*/3,
                        /*factor_cnt*/10);
    FM_Predict pred(train, "./data/train_sparse.csv", true);
#elif defined TEST_GBM
    GBM_Algo_Abst *train = new Train_GBM_Algo(
                          "./data/train_dense.csv",
                          /*epoch*/1,
                          /*maxDepth*/12,
                          /*minLeafHess*/1);
    GBM_Predict pred(train, "./data/train_dense.csv", true);
#elif defined TEST_GMM
    EM_Algo_Abst<vector<double> > *train =
    new Train_GMM_Algo(
                      "./data/train_cluster.csv",
                      /*epoch*/50, /*cluster_cnt*/100,
                      /*feature_cnt*/10);
    T = 1;
#elif defined TEST_TM
    EM_Algo_Abst<vector<vector<double>* > > *train =
    new Train_TM_Algo(
                   "./data/train_topic.csv",
                   "./data/vocab.txt",
                   /*epoch*/50,
                   /*topic*/5,
                   /*word*/5000);
    T = 1;
#elif defined TEST_EMB
    Train_Embed_Algo *train =
    new Train_Embed_Algo(
                       "./data/vocab.txt",
                       "./data/train_text.txt",
                       /*epoch*/50,
                       /*window_size*/6,
                       /*emb_dimention*/100,
                       /*vocab_cnt*/5000);
    T = 1;
#elif defined TEST_CNN
    DL_Algo_Abst<Square<double, int>, Tanh, Softmax> *train =
    new Train_CNN_Algo<Square<double, int>, Tanh, Softmax>(
                         "./data/train_dense.csv",
                         /*epoch*/100,
                         /*feature_cnt*/784,
                         /*hidden_size*/100,
                         /*multiclass_output_cnt*/10);
    T = 1;
#elif defined TEST_VAE
    Train_VAE_Algo<Square<double, double>, Sigmoid> *train =
    new Train_VAE_Algo<Square<double, double>, Sigmoid>(
                         "./data/train_dense.csv",
                         /*epoch*/100,
                         /*feature_cnt*/784,
                         /*hidden*/100,
                         /*gauss*/20);
    T = 1;
#elif defined TEST_RNN
    DL_Algo_Abst<Square<double, int>, Tanh, Softmax> *train =
    new Train_RNN_Algo<Square<double, int>, Tanh, Softmax>(
                         "./data/train_dense.csv",
                         /*epoch*/600,
                         /*feature_cnt*/784,
                         /*hidden_size*/30,
                         /*recurrent_cnt*/28,
                         /*multiclass_output_cnt*/10);
    T = 1;
#elif defined TEST_NFM
    FM_Algo_Abst *train = new Train_NFM_Algo(
                         "./data/train_sparse.csv",
                         /*epoch*/3,
                         /*factor_cnt*/10,
                         /*hidden_layer_size*/10);
    FM_Predict pred(train, "./data/train_sparse.csv", true);
#endif
    
    while (T--) {
        train->Train();
        // Notice whether the algorithm have Predictor
        pred.Predict("");
#ifdef TEST_EMB
        // Notice, word embedding vector multiply 100 to cluster
        EM_Algo_Abst<vector<double> > *cluster =
        new Train_GMM_Algo(
                        "./output/word_embedding.txt",
                        50,
                        50,
                        100,
                        /*scale*/10);
        cluster->Train();
        shared_ptr<vector<int> > ans = cluster->Predict();
        train->EmbeddingCluster(ans, 50);
#endif
        cout << "------------" << endl;
    }
    train->saveModel(0);
    delete train;
    return 0;
}

