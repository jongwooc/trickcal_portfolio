#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <float.h>
#include <math.h>

#include "fully.h"
#include "conv.h"
#include "pool.h"
#include "reshape.h"
#include "variable.h"
#include "image.h"
#include "gemm.h"

//#define FIXED_POINT_ARITHMETIC

openCL_Values * G_test1;

float fLearningRate;

static inline void swap(float **a, float **b) {
    float *tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline void softmax(float *input, int n, float temp, float *output) {
    int i;
    float sum = 0;
    float largest = -FLT_MAX;
    for (i = 0; i < n; i++) {
        if (input[i] > largest)
            largest = input[i];
    }
    for (i = 0; i < n; i++) {
        float e = exp(input[i] / temp - largest / temp);
        sum += e;
        output[i] = e;
    }
    for (i = 0; i < n; i++) {
        output[i] /= sum;
    }
}

static inline char *replace_str(char *str, char *orig, char *rep)
{
    static char buffer[512];
    char *p;

    if(!(p = strstr(str, orig)))
        return str;

    strncpy(buffer, str, p-str);
    buffer[p-str] = '\0';

    sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

    return buffer;
}

#if (0)
static void train_with_images() {
    int fd = open("weight.byte", O_RDWR);
    int i, j, nBatch = 0, num_right = 0, num_wrong = 0;

    struct stat statbuf;
    fstat(fd, &statbuf);
    float *weight =
        (float *)mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    float pfLabel[64];

    float *conv1_weight = weight + 0;
    float *conv1_bias = weight + 500;
    float *conv2_weight = weight + 520;
    float *conv2_bias = weight + 1770;
    float *fc1_weight = weight + 1820;
    float *fc1_bias = weight + 401820;
    float *fc2_weight = weight + 402320;
    float *fc2_bias = weight + 407320;
    
    float pfInput[64 * 784];
    float pfProb[64 * 10];
    float pfDelta[64 * 10];

    float *conv1_data = get_conv1_data();
    float *mp1_data = get_mp1_data();
    float *conv2_data = get_conv2_data();
    float *mp2_data = get_mp2_data();
    float *fc1_data = get_fc1_data();
    float *fc2_data = get_fc2_data();
    
    float *conv1_delta = get_conv1_delta();
    float *mp1_delta = get_mp1_delta();
    float *conv2_delta = get_conv2_delta();
    float *mp2_delta = get_mp2_delta();
    float *fc1_delta = get_fc1_delta();
    float *fc2_delta = get_fc2_delta();
    
   
    char train_im_path[512], train_label_path[512];
    strcpy(train_im_path, "./input/data/train-images.idx3-ubyte"); 
    FILE *pfTrainImg = fopen(train_im_path, "r");
    strcpy(train_label_path, replace_str(train_im_path, "train.txt", "labels/all_label.txt"));
    FILE *pfTrainLabel = fopen(train_label_path, "r");
    
    for (i = 1; i <= 60000; i++) {
        fLearningRate = LEARNING_RATE * pow((1 + GAMMA * i), (-POWER));
       
        char im_path[512];
        float tmpf;
        
        for (nBatch = 0; nBatch < 64; nBatch++) {
            fscanf(pfTrainLabel, "%f %f %f %f %f", &pfLabel[nBatch], &tmpf, &tmpf, &tmpf, &tmpf);
            if ((pfLabel[nBatch] < 0 || pfLabel[nBatch] >= 10)) {            
                fseek(pfTrainLabel, 0, SEEK_SET);
                fseek(pfTrainImg, 0, SEEK_SET);
                fscanf(pfTrainLabel, "%f %f %f %f %f", &pfLabel[nBatch], &tmpf, &tmpf, &tmpf, &tmpf);
                fprintf(stderr, "seek 0 - label : %f\n", pfLabel[nBatch]);
            }

            fgets(im_path, 512, pfTrainImg);
            im_path[strlen(im_path) - 1] = '\0';
            image orig = load_image_color(im_path, 28, 28);
            
            for (j = 0; j < 784; j++)
                pfInput[nBatch * 784 + j] = orig.data[j];
        }
        
        // get the label of the image.
        
        conv1_forward(conv1_weight, conv1_bias, pfInput);
        mp1_forward(conv1_data);
        conv2_forward(conv2_weight, conv2_bias, mp1_data);
        mp2_forward(conv2_data);
        fc1_forward(fc1_weight, fc1_bias, mp2_data);
        fc2_forward(fc2_weight, fc2_bias, fc1_data);
        
        
        int j, k;
        int answer = -1, guess = -1, max = -1;
        for (j = 0; j < 64; j++) {
            softmax(fc2_data + j * 10, 10, 1, pfProb + j * 10);

            answer = (int)pfLabel[j];
            guess = -1;
            max = -1;
            for (k = 0; k < 10; k++) {
                if (max < fc2_data[j * 10 + k]) {
                    max = fc2_data[j * 10 + k];
                    guess = k;
                }

                float diff;
                if (answer == k)
                    diff = 1 - pfProb[j * 10 + k];
                else
                    diff = -pfProb[j * 10 + k];
                pfProb[j * 10 + k] = diff * diff;
                pfDelta[j * 10 + k] = diff;
            }
            if (guess == answer)
                num_right++;
            else
                num_wrong++;
        }
        
	for (j = 0; j < 10 * 64; j++)
            fc2_delta[j] += pfDelta[j];

	if (i % 1 == 0) {
            printf("[ TRAIN | %d / 60000] learning rate : %.2e Acc : %.2f (%d of %d)\n", i, fLearningRate, ((float)num_right) / (num_right + num_wrong) * 100, num_right, num_wrong + num_right);
            //num_right = 0;
            //num_wrong = 0;
        }
        if (i % 100 == 0) {
            num_right = 0;
            num_wrong = 0;
	}

        fc2_backprop(fc2_weight, fc1_data, fc1_delta);
        fc1_backprop(fc1_weight, mp2_data, mp2_delta);
        mp2_backprop(conv2_delta);
        conv2_backprop(conv2_weight, mp1_data, mp1_delta);
        mp1_backprop(conv1_delta);
        conv1_backprop(conv1_weight, pfInput, NULL);
        
        conv1_update(conv1_weight, conv1_bias);
        conv2_update(conv2_weight, conv2_bias);
        fc1_update(fc1_weight, fc1_bias);
        fc2_update(fc2_weight, fc2_bias);
        
    }
    printf("\n");

    fclose(pfTrainImg);
}

static void test_with_images() {
    image orig = load_image_color("giraffe.jpg", 28, 28);
}
#endif

#if (1)
static void train_with_bytes() {
    int fd = open("weight.byte", O_RDWR);
    int i, num_right = 0, num_wrong = 0;

    struct stat statbuf;
    fstat(fd, &statbuf);
    float *weight =
        (float *)mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    float pfLabel[64];

    float *conv1_weight = weight + 0;
    float *conv1_bias = weight + 500;
    float *conv2_weight = weight + 520;
    float *conv2_bias = weight + 1770;
    float *fc1_weight = weight + 1820;
    float *fc1_bias = weight + 401820;
    float *fc2_weight = weight + 402320;
    float *fc2_bias = weight + 407320;
    
    float pfInput[64 * 784];
    float pfProb[64 * 10];
    float pfDelta[64 * 10];

    float *conv1_data = get_conv1_data();
    float *mp1_data = get_mp1_data();
    float *conv2_data = get_conv2_data();
    float *mp2_data = get_mp2_data();
    float *fc1_data = get_fc1_data();
    float *fc2_data = get_fc2_data();
    
    float *conv1_delta = get_conv1_delta();
    float *mp1_delta = get_mp1_delta();
    float *conv2_delta = get_conv2_delta();
    float *mp2_delta = get_mp2_delta();
    float *fc1_delta = get_fc1_delta();
    float *fc2_delta = get_fc2_delta();
    

    FILE *pfTrainImg = fopen("train_image.byte", "r");
    FILE *pfTrainLabel = fopen("train_label.byte", "r");

    for (i = 1; i <= 60000; i++) {
        int nRead;
        fLearningRate = LEARNING_RATE * pow((1 + GAMMA * i), (-POWER));
        nRead = fread(pfLabel, sizeof(float), 64, pfTrainLabel);
        if (nRead < 64) {
            if (fread(pfInput, sizeof(float), nRead * 784, pfTrainImg) != nRead * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
            fseek(pfTrainLabel, 0, SEEK_SET);
            fseek(pfTrainImg, 0, SEEK_SET);
            if (fread(&pfLabel[nRead], sizeof(float), (64 - nRead), pfTrainLabel) != (64 - nRead))
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
            if (fread(&pfInput[nRead * 784], sizeof(float), (64 - nRead) * 784, pfTrainImg) != (64 - nRead) * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
        } else {
            nRead = fread(pfInput, sizeof(float), 64 * 784, pfTrainImg);
            if (nRead != 64 * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
        }

//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
       conv1_forward(conv1_weight, conv1_bias, pfInput);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        mp1_forward(conv1_data);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        conv2_forward(conv2_weight, conv2_bias, mp1_data);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        mp2_forward(conv2_data);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        fc1_forward(fc1_weight, fc1_bias, mp2_data);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        fc2_forward(fc2_weight, fc2_bias, fc1_data);
//        fprintf(stderr, "%s, %d\n", __FILE__, __LINE__);
        
        int j, k;
        int answer = -1, guess = -1, max = -1;
        for (j = 0; j < 64; j++) {
            softmax(fc2_data + j * 10, 10, 1, pfProb + j * 10);

            answer = (int)pfLabel[j];
            guess = -1;
            max = -1;
            for (k = 0; k < 10; k++) {
                if (max < fc2_data[j * 10 + k]) {
                    max = fc2_data[j * 10 + k];
                    guess = k;
                }

                float diff;
                if (answer == k)
                    diff = 1 - pfProb[j * 10 + k];
                else
                    diff = -pfProb[j * 10 + k];
                pfProb[j * 10 + k] = diff * diff;
                pfDelta[j * 10 + k] = diff;
            }
            if (guess == answer)
                num_right++;
            else
                num_wrong++;
        }
        for (j = 0; j < 10 * 64; j++)
            fc2_delta[j] += pfDelta[j];
        for (j = 0; j < 10 * 64; j += 100)
	    printf("%f ", fc2_delta[j]);        
	printf("\n");        


        if (i % 1 == 0) {
            printf("[ TRAIN | %d / 60000] learning rate : %.2e Acc : %.2f (%d of %d)\n", i, fLearningRate, ((float)num_right) / (num_right + num_wrong) * 100, num_right, num_wrong + num_right);
            num_right = 0;
            num_wrong = 0;
        }

        fc2_backprop(fc2_weight, fc1_data, fc1_delta);
        fc1_backprop(fc1_weight, mp2_data, mp2_delta);
        mp2_backprop(conv2_delta);
        conv2_backprop(conv2_weight, mp1_data, mp1_delta);
        mp1_backprop(conv1_delta);
        conv1_backprop(conv1_weight, pfInput, NULL);
        
        conv1_update(conv1_weight, conv1_bias);
        conv2_update(conv2_weight, conv2_bias);
        fc1_update(fc1_weight, fc1_bias);
        fc2_update(fc2_weight, fc2_bias);
        
    }
    printf("\n");

    fclose(pfTrainLabel);
    fclose(pfTrainImg);
}

static void test_with_bytes() {
    int fd = open("weight.byte", O_RDWR);
    int i, num_right, num_wrong;
    
    struct stat statbuf;
    fstat(fd, &statbuf);
    float *weight =
        (float *)mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    float pfLabel[64];

    float *conv1_weight = weight + 0;
    float *conv1_bias = weight + 500;
    float *conv2_weight = weight + 520;
    float *conv2_bias = weight + 1770;
    float *fc1_weight = weight + 1820;
    float *fc1_bias = weight + 401820;
    float *fc2_weight = weight + 402320;
    float *fc2_bias = weight + 407320;
    
    float pfInput[64 * 784];
    float pfProb[64 * 10];

    float *conv1_data = get_conv1_data();
    float *mp1_data = get_mp1_data();
    float *conv2_data = get_conv2_data();
    float *mp2_data = get_mp2_data();
    float *fc1_data = get_fc1_data();
    float *fc2_data = get_fc2_data();
    

    FILE *pfTestImg = fopen("test_image.byte", "r");
    FILE *pfTestLabel = fopen("test_label.byte", "r");

    num_right = 0; num_wrong = 0;
    for (i = 1; i <= 10000; i++) {
        int nRead;
        nRead = fread(pfLabel, sizeof(float), 64, pfTestLabel);
        if (nRead < 64) {
            if (fread(pfInput, sizeof(float), nRead * 784, pfTestImg) != nRead * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
            fseek(pfTestLabel, 0, SEEK_SET);
            fseek(pfTestImg, 0, SEEK_SET);
            if (fread(&pfLabel[nRead], sizeof(float), (64 - nRead), pfTestLabel) != (64 - nRead))
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
            if (fread(&pfInput[nRead * 784], sizeof(float), (64 - nRead) * 784, pfTestImg) != (64 - nRead) * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
        } else {
            nRead = fread(pfInput, sizeof(float), 64 * 784, pfTestImg);
            if (nRead != 64 * 784)
                fprintf(stderr, "fread error at %s, %d\n", __FILE__, __LINE__);
        }

        conv1_forward(conv1_weight, conv1_bias, pfInput);
        mp1_forward(conv1_data);
        conv2_forward(conv2_weight, conv2_bias, mp1_data);
        mp2_forward(conv2_data);
        fc1_forward(fc1_weight, fc1_bias, mp2_data);
        fc2_forward(fc2_weight, fc2_bias, fc1_data);
        
        int j, k;
        int answer = -1, guess = -1, max = -1;
        for (j = 0; j < 64; j++) {
            softmax(fc2_data + j * 10, 10, 1, pfProb + j * 10);

            answer = (int)pfLabel[j];
            guess = -1;
            max = -1;
            for (k = 0; k < 10; k++) {
                if (max < fc2_data[j * 10 + k]) {
                    max = fc2_data[j * 10 + k];
                    guess = k;
                }

                float diff;
                if (answer == k)
                    diff = 1 - pfProb[j * 10 + k];
                else
                    diff = -pfProb[j * 10 + k];
                pfProb[j * 10 + k] = diff * diff;
            }
            if (guess == answer)
                num_right++;
            else
                num_wrong++;
        }
        printf("[ TEST | %d / 10000] learning rate : %.2e Acc : %.2f\r", i, fLearningRate, ((float)num_right) / (num_right + num_wrong) * 100);
    }
    printf("\n");

    fclose(pfTestLabel);
    fclose(pfTestImg);
}
#endif

int main() {
	//openCL_Values * test1;
	G_test1 = calloc(1, sizeof(openCL_Values));
	//G_test1 = test1;

	init_openCL(G_test1);

#if (1)
    // mnist, cifar
    train_with_bytes();
    test_with_bytes();
#endif 

#if (0)
    // VOC
    train_with_images();
    test_with_images();
#endif



	clean_openCL(G_test1);
    return 0;
}
