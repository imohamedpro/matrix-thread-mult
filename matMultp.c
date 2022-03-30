#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#define MAX 100

struct Matrix{
    int ** values;
    int n_rows, n_cols;
} a, b, c_full, c_row, c_element;

typedef struct Element{
    int row;
    int col;
} element;

struct Matrix read_matrix(char * file_name);

void save_matrix(struct Matrix matrix, char * file_name, char * method);

void init_result_matrix(struct Matrix * matrix);

void start_per_matrix(char * output_name);

void start_per_row(char * output_name);

void start_per_element(char * output_name);

void free_memory();

int main(int argc, char* argv[])
{
    char names[3][MAX];
    if(argc == 4){
        for(int i = 0; i < 2; i++) {
            strcpy(names[i], argv[i + 1]);
            strcat(names[i], ".txt");
        }
        strcpy(names[2], argv[3]);
    }else{
        names[0][0] = 'a';
        names[1][0] = 'b';
        strcpy(names[2], "c");
        for(int i = 0; i < 2; i++) {
            names[i][1] = '\0';
            strcat(names[i], ".txt");
        }
    }
    a = read_matrix(names[0]);
    b = read_matrix(names[1]);
    if(a.n_cols != b.n_rows){
        printf("ERROR! Matrices dimensions don't match!\n");
        return 1;
    }
    struct Matrix * matrices[3] = {&c_full, &c_row, &c_element};
    for(int i = 0; i < 3; i++){
        init_result_matrix(matrices[i]);
    }

    start_per_matrix(names[2]);
    start_per_row(names[2]);
    start_per_element(names[2]);

    free_memory();
    return 0;
}

struct Matrix read_matrix(char * file_name){
    FILE * file = fopen(file_name, "r");
    char c;
    char num[10];
    int count = 0, n_rows = 0, n_cols = 0;
    do{
        c = (char) fgetc(file);
        if(isdigit(c)){
            *(num+count) = c;
            count++;
            *(num+count) = '\0';
        }
        else if(isspace(c)){
            if(n_rows){
                n_cols = (int) strtol(num, NULL, 10);
            }else{
                n_rows = (int) strtol(num, NULL, 10);
            }
            count = 0;
        }
    } while (c != '\n');

    int ** matrix = malloc(n_rows * sizeof(int*));
    for(int i = 0; i < n_rows; i++){
        matrix[i] = malloc(n_cols * sizeof(int));
    }
    for(int i = 0; i < n_rows; ++i)
    {
        for(int j = 0; j < n_cols; ++j)
            fscanf(file, "%d", matrix[i]+j);
    }
    struct Matrix m;
    m . values = matrix;
    m . n_rows = n_rows;
    m . n_cols = n_cols;
    return m;
}

void * matrix_per_thread(){
    for(int i = 0; i < a.n_rows; i++){
        for(int j = 0; j < b.n_cols; j++){
            c_full.values[i][j] = 0;
            for(int k = 0; k < a.n_cols; k++){
                c_full.values[i][j] += a.values[i][k] * b.values[k][j];
            }
        }
    }
}

void * matrix_per_row(void * m){
    long row = ((long) m);
    row--;  //decrementing row because we started the counter with 1
    for(int i = 0; i < b.n_cols; i++){
        c_row.values[row][i] = 0;
        for(int j = 0; j < b.n_rows; j++){
            c_row.values[row][i] += a.values[row][j] * b.values[j][i];
        }
    }
}

void * matrix_per_element(void * m){
    element* e = (element*) m;
    c_element.values[e->row][e->col] = 0;
    for(int i = 0; i < a.n_cols; i++){
        c_element.values[e->row][e->col] += a.values[e->row][i] * b.values[i][e->col];
    }
    free(e);
}

void start_per_matrix(char * output_name){
    printf("Method: Thread Per Matrix\n");
    struct timeval stop, start;
    pthread_t thread;
    gettimeofday(&start, NULL);
    pthread_create(&thread, NULL, matrix_per_thread, NULL);
    pthread_join(thread, NULL);
    gettimeofday(&stop, NULL);
    printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("\tMicroseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    char file_name[MAX];
    strcpy(file_name, output_name);
    strcat(file_name, "_per_matrix.txt");
    save_matrix(c_full, file_name,"A thread per matrix");
}

void start_per_row(char * output_name){
    printf("Method: Thread Per Row\n");
    struct timeval stop, start;
    pthread_t thread_row[a.n_rows];
    gettimeofday(&start, NULL);
    for(long i = 1; i < a.n_rows+1; i++)    //i = 1 because when casting 0 to (void*) it returns NULL
        pthread_create(&thread_row[i-1], NULL, matrix_per_row, (void *)i);
    for(int i = 0; i < a.n_rows; i++)
        pthread_join(thread_row[i], NULL);
    gettimeofday(&stop, NULL);
    printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("\tMicroseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    char file_name[MAX];
    strcpy(file_name, output_name);
    strcat(file_name, "_per_row.txt");
    save_matrix(c_row, file_name,"A thread per row");
}

void start_per_element(char * output_name){
    printf("Method: Thread Per Element\n");
    struct timeval stop, start;
    pthread_t thread_element[a.n_rows * b.n_cols];

    gettimeofday(&start, NULL);
    for(int i = 0; i < a.n_rows; i++){
        for(int j = 0; j < b.n_cols; j++){
            element * e = malloc(sizeof(element));
            (*e).row = i;
            (*e).col = j;
            pthread_create(&thread_element[i * b.n_cols + j], NULL, matrix_per_element, (void *)e);
        }
    }
    for(int i = 0; i < a.n_rows * b.n_cols; i++)
        pthread_join(thread_element[i], NULL);
    gettimeofday(&stop, NULL);
    printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("\tMicroseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    char file_name[MAX];
    strcpy(file_name, output_name);
    strcat(file_name, "_per_element.txt");
    save_matrix(c_element, file_name,"A thread per element");
}

void init_result_matrix(struct Matrix * matrix){
    (*matrix).n_rows = a.n_rows;
    (*matrix).n_cols = b.n_cols;
    (*matrix).values = malloc(c_full.n_rows * sizeof(int*));
    for(int i = 0; i < (*matrix).n_rows; i++){
        (*matrix).values[i] = malloc((*matrix).n_cols * sizeof(int));
    }
}

void save_matrix(struct Matrix matrix, char * file_name, char * method){
    FILE * file = fopen(file_name, "w");
    fprintf(file, "Method: %s\nrow=%d col=%d\n", method, matrix.n_rows, matrix.n_cols);
    for(int i=0; i < matrix.n_rows; i++){
        for(int j=0; j < matrix.n_cols; j++){
            fprintf(file,"%d ", matrix.values[i][j]);
        }
        fprintf(file,"\n");
    }
}

void free_memory(){
    struct Matrix matrices[] = {a, b, c_full, c_element, c_row};
    for(int i = 0; i < 5 ; i++){
        for(int j = 0; j < matrices[i].n_rows; j++){
                free(matrices[i].values[j]);
        }
        free(matrices[i].values);
    }
}
