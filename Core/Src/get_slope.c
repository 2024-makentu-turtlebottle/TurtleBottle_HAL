/*
 * get_slope.c
 *
 *  Created on: May 5, 2024
 *      Author: czli
 */
#include "get_slope.h"

unsigned char data[HEIGHT][WIDTH][CHANNELS];
unsigned char dst[HEIGHT][WIDTH] = {0};

/* Transpose of A Buffer */
float32_t AT_f32[NUM_DATA_MAX * 2];
/* (Transpose of A * A) Buffer */
float32_t ATMA_f32[4];
/* Inverse(Transpose of A * A)  Buffer */
float32_t ATMAI_f32[4];

float32_t ATMAIAT_f32[NUM_DATA_MAX * 2];


/* Test Output Buffer */
float32_t B_f32[NUM_DATA_MAX];
float32_t A_f32[NUM_DATA_MAX][2];
int num_data;

float32_t X_f32[2];

Queue q;

void clearQueue() {
    q.front = -1;
    q.rear = -1;
}

// 檢查隊列是否為空
bool isEmpty() {
    return q.rear == -1;
}

// 將元素放入隊列
void enqueue(Point item) {
    if (q.rear == WIDTH * HEIGHT - 1)
        return;
    if (q.front == -1)
        q.front = 0;
    q.rear++;
    q.items[q.rear] = item;
}

// 從隊列中取出元素
Point dequeue() {
    Point item = {-1, -1};
    if (isEmpty())
        return item;
    item = q.items[q.front];
    q.front++;
    if (q.front > q.rear)
        q.front = q.rear = -1;
    return item;
}

// 計算兩個像素之間的歐式距離
float calculateDistanceSquared(unsigned char a[CHANNELS], unsigned char b[CHANNELS]) {
    float distance = 0.0;
    for (int i = 0; i < CHANNELS; i++) {
        distance += pow(a[i] - b[i], 2);
    }
    return distance;
}

// BFS算法
//void bfs(unsigned char data[HEIGHT][WIDTH][CHANNELS], unsigned char dst[HEIGHT][WIDTH], Point start) {
void bfs() {
	Point start = {WIDTH / 2 - 1, HEIGHT / 2 - 1};

	// Clear dst array
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            dst[i][j] = 0;
        }
    }
    clearQueue();

    int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    enqueue(start);

    while (!isEmpty()) {
        Point current = dequeue();
        for (int i = 0; i < 4; i++) {
            int next_x = current.x + dirs[i][0];
            int next_y = current.y + dirs[i][1];
            if (next_x >= 0 && next_x < WIDTH && next_y >= 0 && next_y < HEIGHT) {
                if (dst[next_y][next_x] == 0 && calculateDistanceSquared(data[next_y][next_x], data[current.y][current.x]) < (DISTANCE * DISTANCE)) {
                    dst[next_y][next_x] = 1;
                    Point next_point = {next_x, next_y};
                    enqueue(next_point);
                }
            }
        }
    }
}

//def get_edge(img):
//  height, width = 160, 120
//  edgesL = []
//  for i in range(height):
//    for j in range(width - 1):
//      if img[i, j] == 0 and img[i, j+1] == 1:
//        edgesL.append([j, i])
//        break
//
//  return np.array(edgesL)

void get_edge() {
	num_data = 0;

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH - 1; j++) {
			if (dst[i][j] == 0 && dst[i][j+1] == 1) {
				A_f32[num_data][0] = i;
				A_f32[num_data][1] = 1;

				B_f32[num_data] = j;
				num_data += 1;
				break;
			}
		}
		if (num_data == NUM_DATA_MAX) {
			break;
		}
	}

}

float32_t get_slope() {
	bfs();
	get_edge();

	arm_status status;

	arm_matrix_instance_f32 A;      /* Matrix A Instance */
	arm_matrix_instance_f32 AT;     /* Matrix AT(A transpose) instance */
	arm_matrix_instance_f32 ATMA;   /* Matrix ATMA( AT multiply with A) instance */
	arm_matrix_instance_f32 ATMAI;  /* Matrix ATMAI(Inverse of ATMA) instance */
	arm_matrix_instance_f32 B;      /* Matrix B instance */
	arm_matrix_instance_f32 X;      /* Matrix X(Unknown Matrix) instance */

	arm_matrix_instance_f32 ATMAIAT;

	arm_mat_init_f32(&A, num_data, 2, (float32_t *)A_f32);

	arm_mat_init_f32(&AT, 2, num_data, AT_f32);
	status = arm_mat_trans_f32(&A, &AT);

	arm_mat_init_f32(&ATMA, 2, 2, ATMA_f32);
	status = arm_mat_mult_f32(&AT, &A, &ATMA);

	arm_mat_init_f32(&ATMAI, 2, 2, ATMAI_f32);
	status = arm_mat_inverse_f32(&ATMA, &ATMAI);

	arm_mat_init_f32(&ATMAIAT, 2, num_data, ATMAIAT_f32);
	status = arm_mat_mult_f32(&ATMAI, &AT, &ATMAIAT);

	arm_mat_init_f32(&B, num_data, 1, (float32_t *)B_f32);

	arm_mat_init_f32(&X, 2, 1, X_f32);

	status = arm_mat_mult_f32(&ATMAIAT, &B, &X);

	return X_f32[0];
}
