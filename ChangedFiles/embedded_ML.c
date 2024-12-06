
/**
 ******************************************************************************
 * @file    embedded_M.c
 * @author  UCLA IoT Team
 * @brief   This code includes the functions enabling EmbeddedML developed by
 *          Charles Zaloom
 *          https://github.com/merrick7/EmbeddedML
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include <math.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>


//serial setup start
//serial headers
#include "embedded_ML.h"
#include <string.h>
#include "stm32u5xx.h"
#include "stm32u5xx_hal.h"

UART_HandleTypeDef huart1;

void UART1_Init(void) {
    // Enable UART and GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    // Configure GPIO pins for UART (PA9 TX, PA10 RX)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure UART
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;

    // Initialize UART
    HAL_UART_Init(&huart1);
}

void UART_SendChar(uint8_t c) {
    HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
}

void UART_SendString(char* str) {
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

void UART_ReceiveString(char* buffer, uint16_t bufferSize) {
    HAL_UART_Receive(&huart1, (uint8_t*)buffer, bufferSize, HAL_MAX_DELAY);
}


//serial setup end remove



void Feature_Extraction_State_Machine_Translation(float * feature_1,
		float * feature_2, float * sig_mag_scale) {



	float acc_x, acc_y;
	float acc_x_prev, acc_y_prev;
	float acc_x_filter_lp, acc_y_filter_lp;
	float acc_x_filter_hp;
	float acc_y_filter_hp;
	float acc_x_filter_hp_prev, acc_y_filter_hp_prev;
	float acc_x_filter_lp_prev, acc_y_filter_lp_prev;
	float vel_x = 0;
	float vel_x_prev = 0;
	float vel_y = 0;
	float vel_y_prev = 0;
	float vel_mag;
	float sample_period;
	float W_HP, I_HP, iir_HP_0, iir_HP_1, iir_HP_2;
	float W_LP, I_LP, iir_LP_0, iir_LP_1, iir_LP_2;
	float integration_interval;
	int32_t motion_state;
	float velocity_mag_threshold;
	float velocity_diff_threshold;

	int32_t result;

	BSP_MOTION_SENSOR_Axes_t axes;


	/*
	 * Define sample period
	 */
	sample_period = INTEGRATION_INTERVAL_DISP;

	/*
	 * Set velocity magnitude threshold value (in millimeters/sec) for recognition of motion state.
	 */

	velocity_mag_threshold = 40;

	/*
	 * Set velocity difference threshold value (in millimeters/sec) for recognition of motion state
	 * termination by detection of no change between successive velocity samples.
	 */

	velocity_diff_threshold = 0.01;

	/*
	 * Compute high pass filter coefficients
	 */

	W_HP = 2 * 3.1416 * HIGH_PASS_FILTER_FREQUENCY_DISP;
	I_HP = 2/(W_HP * sample_period);
	if (HP_FILTER_ENABLE_DISP == 1){
		iir_HP_0 = 1 - (1/(1 + I_HP));
		iir_HP_1 = -iir_HP_0;
		iir_HP_2 = (1/(1 + I_HP))*(1 - I_HP);
	}
	if (HP_FILTER_ENABLE_DISP == 0){
		iir_HP_0 = 1;
		iir_HP_1 = 0;
		iir_HP_2 = 0;
	}

	/*
	 * Compute low pass filter coefficients
	 */

	W_LP = 2 * 3.1416 * LOW_PASS_FILTER_FREQUENCY_DISP;
	I_LP = 2/(W_LP * sample_period);
	if (LP_FILTER_ENABLE_DISP == 1){
		iir_LP_0 = 1/(1 + I_LP);
		iir_LP_1 = iir_LP_0;
		iir_LP_2 = (1/(1 + I_LP))*(1 - I_LP);
	}
	if (LP_FILTER_ENABLE_DISP == 0){
		iir_LP_0 = 1;
		iir_LP_1 = 0;
		iir_LP_2 = 0;
	}

	printf("Start Motion Segment with Green LED Blink\n");

	//HAL_Delay(2000);

	BSP_LED_Off(LED6);
	LED_Notification_Blink_Green(5);
	BSP_LED_On(LED7);

	motion_state = 0;
	acc_x = 0;
	acc_y = 0;
	acc_x_prev = 0;
	acc_y_prev = 0;
	acc_x_filter_lp_prev = 0;
	acc_y_filter_lp_prev = 0;
	acc_x_filter_hp_prev = 0;
	acc_y_filter_hp_prev = 0;

	vel_x = 0;
	vel_x_prev = 0;
	vel_y = 0;
	vel_y_prev = 0;

	while (1) {

		/*
		 * Introduce delay establishing integration interval
		 * This is summed with sensor data access delay to
		 * yield loop delay of 4 msec
		 */

		HAL_Delay(2);

		result = 0;
		if (BSP_MOTION_SENSOR_GetAxes(0, MOTION_ACCELERO, &axes) != BSP_ERROR_NONE) result--;
		if (result < 0){
			printf("Sampling Error %i\n", result);
		}



		/* Setting for integration interval */

		integration_interval = INTEGRATION_INTERVAL_DISP;

		/* Convert acceleration to millimeter/sec/sec units sing 1 milli-g = (9.81 m/s/s)/1000 */

		acc_x = 9.81*(float)(axes.xval);
		acc_y = 9.81*(float)(axes.yval);


		/* Apply Low Pass filter to acceleration */
		acc_x_filter_lp  = iir_LP_0 * acc_x + iir_LP_1 * acc_x_prev - iir_LP_2 * acc_x_filter_lp_prev;
		acc_y_filter_lp  = iir_LP_0 * acc_y + iir_LP_1 * acc_y_prev - iir_LP_2 * acc_y_filter_lp_prev;

		/* Apply High Pass filter to signal of Low Pass filter to acceleration */
		acc_x_filter_hp = iir_HP_0 * acc_x_filter_lp  + iir_HP_1 * acc_x_filter_lp_prev - iir_HP_2 * acc_x_filter_hp_prev;
		acc_y_filter_hp = iir_HP_0 * acc_y_filter_lp  + iir_HP_1 * acc_y_filter_lp_prev - iir_HP_2 * acc_y_filter_hp_prev;

		acc_x_prev = acc_x;
		acc_y_prev = acc_y;

		/*
		 * Compute velocity by integration
		 */

		vel_x = vel_x + (acc_x_filter_hp + acc_x_filter_hp_prev) * integration_interval/2;
		vel_y = vel_y + (acc_y_filter_hp + acc_y_filter_hp_prev) * integration_interval/2;

		acc_x_filter_lp_prev = acc_x_filter_lp;
		acc_y_filter_lp_prev = acc_y_filter_lp;

		acc_x_filter_hp_prev = acc_x_filter_hp;
		acc_y_filter_hp_prev = acc_y_filter_hp;

		/*
		 * Detect motion and assign velocities to features
		 */

		vel_mag = sqrt((vel_x)*(vel_x) + (vel_y)*(vel_y));

		if (motion_state == 0 && vel_mag > velocity_mag_threshold){
			motion_state = 1;
			BSP_LED_Off(LED7);
			*feature_1 = vel_x;
			*feature_2 = vel_y;

			*sig_mag_scale = vel_mag;
		}

		/*
		 * Detect termination of motion
		 */

		if (motion_state == 1 && abs(vel_x_prev - vel_x) < velocity_diff_threshold &&
				abs(vel_y_prev - vel_y) < velocity_diff_threshold){

			BSP_LED_Off(LED7);
			LED_Notification_Blink_Red(5);
			break;
		}

		vel_x_prev = vel_x;
		vel_y_prev = vel_y;
	}
	return;
}

void TrainOrientation_SM_Translation(ANN *net) {
	float training_dataset[8][8][NUM_FEATURES];
	float sig_xyz[NUM_FEATURES];
	float sig_xyz_norm[NUM_FEATURES];
	float feature_array[NUM_FEATURES];

	int num_train_data_cycles;
	int training_cycles;
	int i, j, k, m, n, r, error, net_error;
	float feature_1, feature_2, sig_mag_scale;

	float Z_Threshold;

	Z_Threshold = 1.2;

	printf("\n\n\nTraining Start in 2 seconds ..");
	BSP_LED_Off(LED6);
	BSP_LED_Off(LED7);
	HAL_Delay(2000);

	/*
	 * Maximum of 8 cycles
	 */
	num_train_data_cycles = 1;

	for (k = 0; k < num_train_data_cycles; k++) {
		for (i = 0; i < 6; i++) {

			LED_Notification_Blink_Red(3);
			LED_Notification_Blink_Green(3);
			LED_Notification_Blink_Red(3);
			BSP_LED_On(LED6);

			switch (i) {

			case 0:
				printf("\nTrain Start Game Orientation\n");
				break;

			case 1:
				printf("\n\nTrain Pause Game Orientation\n");
				break;

			case 2:
				printf("\n\nTrain Stop Game Orientation\n");
				break;

			case 3:
				printf("\n\nTrain  Square Orientation\n");
				break;

			case 4:
				printf("\n\nTrain Triangle Orientation\n");
				break;

			case 5:
				printf("\n\nTrain Line Orientation\n");
				break;
			}


			/* Motions include 5 segments and 10 associated features */

			n = 0;
			while (n < NUM_FEATURES){
				Feature_Extraction_State_Machine_Translation(&feature_1, &feature_2, &sig_mag_scale);
				feature_array[n] = feature_1;
				n = n + 1;
				printf("Features %0.1f\t: ", feature_array[n]);
				feature_array[n] = feature_2;
				printf("Features %0.1f\t: ", feature_array[n]);
				n = n + 1;
			}

			printf("\n");


			for (n = 0; n < NUM_FEATURES; n++){
				sig_xyz[n] = feature_array[n];
			}


			motion_softmax(net->topology[0], sig_xyz, sig_xyz_norm);

			for (n = 0; n < NUM_FEATURES; n++){
				training_dataset[i][k][n] = sig_xyz_norm[n];
			}

			printf("\n Softmax Input \t");

			for (r = 0; r < NUM_FEATURES; r++) {
				printf("\t%0.1f", sig_xyz[r]);
			}
			printf("\n Softmax Output\t");

			for (r = 0; r < NUM_FEATURES; r++) {
				printf("\t%0.1f", sig_xyz_norm[r]);

			}
			printf("\n\n");

		}
	}

	/*
	 * Enter NN training
	 */

	float Motion_1[6] = { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	float Motion_2[6] = { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
	float Motion_3[6] = { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 };
	float Motion_4[6] = { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
	float Motion_5[6] = { 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
	float Motion_6[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };

	printf("\n\nTraining Start\n");

	training_cycles = TRAINING_CYCLES;

	for (k = 0; k < num_train_data_cycles; k++) {

		i = 0;
		while (i < training_cycles) {
			for (j = 0; j < 6; j++) {

				if ((i % 20 == 0 && i < 100) || i % 100 == 0) {
					printf("\n\nTraining Epochs: %i\n", i);

					LED_Code_Blink_Green(0);

					net_error = 0;
					for (m = 0; m < 6; m++) {
						run_ann(net, training_dataset[m][k]);
						printOutput_ANN_Threshold(net, m, &error, &Z_Threshold);
						if (error == 1) {
							net_error = 1;
						}
					}
					printf("\nError State: %i\n", net_error);

					if (net_error == 0) {
						return;
					}

				}

				switch (j) {

				case 0:
					train_ann(net, training_dataset[j][k], Motion_1);
					break;
				case 1:
					train_ann(net, training_dataset[j][k], Motion_2);
					break;
				case 2:
					train_ann(net, training_dataset[j][k], Motion_3);
					break;
				case 3:
					train_ann(net, training_dataset[j][k], Motion_4);
					break;
				case 4:
					train_ann(net, training_dataset[j][k], Motion_5);
					break;
				case 5:
					train_ann(net, training_dataset[j][k], Motion_6);
					break;
				default:
					break;
				}
				i++;
				HAL_Delay(5);
			}

		}

	}


	if (net_error == 0){
		LED_Code_Blink_Green(0);
		LED_Code_Blink_Green(0);
	} else {
		LED_Code_Blink_Red(1);
		LED_Code_Blink_Red(1);
	}

	printf("\n\nTraining Complete, Now Start Test Motions\n");
	return;
}

void Test_ANN_SM_Translation(ANN *net, int32_t *loc) {

	float point;
	float sig_xyz_norm[NUM_FEATURES];
	float sig_xyz[NUM_FEATURES];
	float feature_1, feature_2, sig_mag_scale;
	float feature_array[NUM_FEATURES];
	int i, n, r;
	int error;
	float Z_Threshold;

	Z_Threshold = 1.2;

	BSP_LED_Off(LED6);
	BSP_LED_Off(LED7);

	LED_Notification_Blink_Red(3);
	LED_Notification_Blink_Green(3);
	LED_Notification_Blink_Red(3);
	BSP_LED_On(LED6);


//	// Prompt the user to perform a motion
//	//printf("\nPlease perform Motion x now.\n");
//	srand(time(NULL)); // Seed the random number generator
//	int roll = rand() % 6 + 1; // Generate a random number between 1 and 6
//	printf("\nNow, please perform motion %d\n", roll);
//	LED_Notification_Blink_Green(3);
//	//HAL_Delay(5000);  // Wait 5 seconds for motion execution
//	//dont' need rand since the game is "randomly" sending down shapes anyways :)


	n = 0;
	while (n < NUM_FEATURES){
		Feature_Extraction_State_Machine_Translation(&feature_1, &feature_2, &sig_mag_scale);
		feature_array[n] = feature_1;
		n = n + 1;
		printf("Features %0.1f\t: ", feature_array[n]);
		feature_array[n] = feature_2;
		printf("Features %0.1f\t: ", feature_array[n]);
		n = n + 1;
	}

	printf("\n");

	for (n = 0; n < NUM_FEATURES; n++){
		sig_xyz[n] = feature_array[n];
	}

	motion_softmax(net->topology[0], sig_xyz, sig_xyz_norm);

	printf("\n Softmax Input \t");

	for (r = 0; r < NUM_FEATURES; r++) {
		printf("\t%0.1f", sig_xyz[r]);
	}
	printf("\n Softmax Output\t");

	for (r = 0; r < NUM_FEATURES; r++) {
		printf("\t%0.3f", sig_xyz_norm[r]);

	}
	printf("\n\n");

	run_ann(net, sig_xyz_norm);


	point = 0.0;

	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		if (net->output[i] > point && net->output[i] > 0.1) {
			point = net->output[i];
			*loc = i;
		}
	}

	printOutput_ANN_Threshold(net, *loc, &error, &Z_Threshold);


	if (error == 1){
		*loc = -1;
	}

	switch (*loc) {
	case 0:
		//printf("\n\rNeural Network Classification - Motion 1");
		printf("\n\rStart game action registered"); //5 moves right
		UART_SendString("1!");
		break;
	case 1:
		//printf("\n\rNeural Network Classification - Motion 2");
		printf("\n\rPause game action registered"); //5 moves left
		UART_SendString("2!");
		break;
	case 2:
		//printf("\n\rNeural Network Classification - Motion 3");
		printf("\n\rStop game action registered"); //5 moves right left right left right
		UART_SendString("3!");
		break;
	case 3:
		//printf("\n\rNeural Network Classification - Motion 4");
		printf("\n\rSquare shape registered"); //start at upper left and trace our circle. trace out top side twice
		UART_SendString("4!");
		//serial.print
		break;
	case 4:
		//printf("\n\rNeural Network Classification - Motion 5");
		printf("\n\rLine registered"); // move right move left move right move left move right
		UART_SendString("5!");
		break;
	case 5:
		printf("\n\r");
		UART_SendString("6!");
		break;
	case -1:
		printf("\n\rNeural Network Classification - ERROR");
		break;
	default:
		printf("\n\rNeural Network Classification - NULL");
		break;
	}

	printf("\n\n\n");

	if (*loc == -1){
		LED_Code_Blink_Red(0);
	} else {
		LED_Code_Blink_Green(*loc + 1);
	}

}






void LED_Code_Blink_Red(int count) {

	int i;

	/*
	 * Alert signal of rapid blinks indicating code to be shown
	 */
	for (i = 0; i < 7; i++) {
		BSP_LED_On(LED6);

		HAL_Delay(20);
		BSP_LED_Off(LED6);

		HAL_Delay(50);
	}

	/*
	 * Code indicated by number of slow blinks
	 */

	if (count != 0) {
		HAL_Delay(1000);
		for (i = 0; i < count; i++) {
			BSP_LED_On(LED6);

			HAL_Delay(500);
			BSP_LED_Off(LED6);

			HAL_Delay(500);
		}
	}

	/*
	 * Alert signal of rapid blinks indicating end of code
	 */
	for (i = 0; i < 7; i++) {
		BSP_LED_On(LED6);

		HAL_Delay(20);
		BSP_LED_Off(LED6);

		HAL_Delay(30);
	}

}

void LED_Code_Blink_Green(int count) {

	int i;

	/*
	 * Alert signal of rapid blinks indicating code to be shown
	 */
	for (i = 0; i < 7; i++) {

		BSP_LED_On(LED7);
		HAL_Delay(20);

		BSP_LED_Off(LED7);
		HAL_Delay(50);
	}

	/*
	 * Code indicated by number of slow blinks
	 */

	if (count != 0) {
		HAL_Delay(1000);
		for (i = 0; i < count; i++) {

			BSP_LED_On(LED7);
			HAL_Delay(500);

			BSP_LED_Off(LED7);
			HAL_Delay(500);
		}
	}

	/*
	 * Alert signal of rapid blinks indicating end of code
	 */
	for (i = 0; i < 7; i++) {

		BSP_LED_On(LED7);
		HAL_Delay(20);

		BSP_LED_Off(LED7);
		HAL_Delay(30);
	}

}

void printOutput_ANN_Threshold(ANN *net, int input_state, int * error, float * Z_Threshold) {

	int i, loc, count;
	float point = 0.0;
	float rms_output, mean_output, mean_output_rem, next_max;
	float classification_metric;

	/*
	 * Initialize error state
	 */

	*error = 0;
	loc = -1;
	count = 0;
	mean_output = 0;
	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		mean_output = mean_output + (net->output[i]);
		if (net->output[i] > point && net->output[i] > 0.1) {
			point = net->output[i];
			loc = i;
		}
		count++;
	}

	next_max = 0;
	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		if (i == loc) {
			continue;
		}
		if (net->output[i] > next_max && net->output[i] > 0.1) {
			next_max = net->output[i];
		}
	}

	mean_output = (mean_output) / (count);

	count = 0;
	mean_output_rem = 0;
	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		mean_output_rem = mean_output_rem + (net->output[i]);
		if (i == loc) {
			continue;
		}
		count++;
	}

	mean_output_rem = (mean_output_rem) / (count);

	rms_output = 0;

	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		rms_output = rms_output + pow((net->output[i] - mean_output), 2);
	}

	rms_output = sqrt(rms_output / count);
	if (rms_output != 0) {
		classification_metric = (point - mean_output) / rms_output;
	} else {
		classification_metric = 0;
	}

	if (loc != input_state) {
		rms_output = 0;
		classification_metric = 0;
		point = 0;
		mean_output = 0;
		mean_output_rem = 0;
	}
	if (loc > -1){
		printf("State: %i",loc);
	}
	if (loc == -1){
		printf("State:  ");
	}

	if (mean_output >= 0){
		printf(" Max: %0.1f Mean:  %0.1f Z: %0.1f ", loc, point, mean_output, classification_metric);
	}
	if (mean_output < 0){
		printf(" Max: %0.1f Mean: %0.1f Z: %0.1f ", loc, point, mean_output, classification_metric);
	}

	printf("Outputs: ");
	for (i = 0; i < net->topology[net->n_layers - 1]; i++) {
		if (net->output[i] >= 0){
			printf("  %0.1f", net->output[i]);
		}
		if (net->output[i] < 0){
			printf(" %0.1f", net->output[i]);
		}
	}

	if (loc != input_state) {
		*error = 1;
		printf("\t Classification Error");
	}

	if (loc == input_state
			&& ( classification_metric < *Z_Threshold
					|| point/next_max < CLASSIFICATION_DISC_THRESHOLD)) {
		*error = 1;
		printf("\t Classification Accuracy Limit");
	}

	printf("\n");
}



void motion_softmax(int n, float *x, float *y) {
	float norm;
	int32_t i;

	norm = 0;
	for (i = 0; i < n; i++){
		norm = norm + x[i] * x[i];
	}
	norm = sqrt(norm);
	for (i = 0; i < n; i++){
		y[i] = x[i]/norm;
	}
}


void LED_Notification_Blink_Green(int count) {

	int i;

	/*
	 * Rapid blink notification
	 */

	for (i = 0; i < count; i++) {

		BSP_LED_On(LED7);
		HAL_Delay(20);

		BSP_LED_Off(LED7);
		HAL_Delay(50);
	}
}

void LED_Notification_Blink_Red(int count) {

	int i;

	/*
	 * Rapid blink notification
	 */

	for (i = 0; i < count; i++) {
		BSP_LED_On(LED6);

		HAL_Delay(20);
		BSP_LED_Off(LED6);

		HAL_Delay(50);
	}
}

void BP_ANN(ANN *net, float *input, float *output, float *weights, float *velocity, float *bias, float *delta, int depth){
	unsigned int i,j;
	unsigned int DIM[2] = {net->topology[net->n_layers - depth], net->topology[net->n_layers - depth - 1]};

	if(depth == 1){
		for(i = 0; i < DIM[0]; i++){
			net->output[i] = 0.0;
			for(j = 0; j < DIM[1]; j++){
				net->output[i] += weights[(DIM[1]*i)+j]*input[j];
			}
			net->output[i] = net->output[i] + bias[i];
			delta[i] = (output[i]-net->output_activation_function(net->output[i])) * net->output_activation_derivative(net->output[i]);
			net->output[i] = net->output_activation_function(net->output[i]);
			bias[i] = bias[i] + delta[i]*net->beta;
		}

		float dEdW[DIM[0]*DIM[1]];
		for(i = 0; i < DIM[0]; i++){
			for(j = 0; j < DIM[1]; j++){
				dEdW[(DIM[1]*i)+j] = delta[i]*input[j];
			}
		}
		for(i = 0; i < DIM[0]*DIM[1]; i++){
			velocity[i] = dEdW[i]*net->eta - velocity[i]*net->alpha;
			weights[i] = weights[i] + velocity[i];
		}
		return;
	}
	else{
		float a[DIM[0]];
		float d[DIM[0]];

		for(i = 0; i < DIM[0]; i++){
			a[i] = 0.0;
			for(j = 0; j < DIM[1]; j++){
				a[i] += weights[(DIM[1]*i)+j]*input[j];
			}
			a[i] += bias[i];
			d[i] = net->hidden_activation_derivative(a[i]);
			a[i] = net->hidden_activation_function(a[i]);
		}

		unsigned int DIM1 = net->topology[net->n_layers - depth + 1];

		float prev_delta[DIM1];
		unsigned int weight_iter = DIM[0] * DIM[1];

		float next_weights_T[DIM[0]*DIM1];
		unsigned int iter = 0;
		for(i = 0; i < DIM[0]; i++){
			for(j = 0; j < DIM1; j++){
				next_weights_T[iter] = weights[(DIM[0]*j)+i+weight_iter];
				iter++;
			}
		}

		BP_ANN(net, a, output, &weights[weight_iter], &velocity[weight_iter], &bias[DIM[0]], prev_delta, depth-1);

		for(i = 0; i < DIM[0]; i++){
			delta[i] = 0;
			for(j = 0; j < DIM1; j++){
				delta[i] += next_weights_T[(DIM1*i)+j]*prev_delta[j];
			}
			delta[i] = delta[i]*d[i];
			bias[i] = bias[i] + delta[i]*net->beta;
		}
		float dEdW[DIM[0]*DIM[1]];
		for(i = 0; i < DIM[0]; i++){
			for(j = 0; j < DIM[1]; j++){
				dEdW[(DIM[1]*i)+j] = delta[i]*input[j];
			}
		}
		for(i = 0; i < DIM[0]*DIM[1]; i++){
			velocity[i] = dEdW[i]*net->eta - velocity[i]*net->alpha;
			weights[i] = weights[i] + velocity[i];
		}
		return;
	}
}

//-----ANN-----


void train_ann(ANN *net, float *input, float *output){
	float delta[net->topology[1]];
	BP_ANN(net, input, output, net->weights, net->dedw, net->bias, delta, net->n_layers-1);
}

void FP_ANN(ANN *net, float *input, unsigned int depth, float *weights){
	unsigned int DIM[2] = {net->topology[net->n_layers - depth], net->topology[net->n_layers - depth - 1]};
	unsigned int i,k;

	if(depth == 1){
		for(i = 0; i < DIM[0]; i++){
			net->output[i] = 0.0;
			for(k = 0; k < DIM[1]; k++){
				net->output[i] += weights[(DIM[1]*i)+k]*input[k];
			}
			net->output[i] = net->output_activation_function(net->output[i] + net->bias[i]);
		}
		return;
	}
	else{
		float a[DIM[0]];
		for(i = 0; i < DIM[0]; i++){
			a[i] = 0.0;
			for(k = 0; k < DIM[1]; k++){
				a[i] += weights[(DIM[1]*i)+k]*input[k];
			}
			a[i] = net->hidden_activation_function(a[i] + net->bias[i]);
			//if(depth == 2) printf("%f,", a[i]);
		}
		//if(depth == 2) printf("0\n");
		FP_ANN(net, a, depth-1, &weights[DIM[0]*DIM[1]]);
	}
	return;
}

void run_ann(ANN *net, float *input){
	FP_ANN(net, input, net->n_layers-1, net->weights);
}

void init_ann(ANN *net){
	fill_number(net->bias, net->n_bias, 0.1);
	fill_zeros(net->dedw, net->n_weights);

	if(net->output_activation_function == &relu) net->output_activation_derivative = &relu_derivative;
	else if(net->output_activation_function == &relu2) net->output_activation_derivative = &relu2_derivative;

	if(net->hidden_activation_function == &relu) net->hidden_activation_derivative = &relu_derivative;
	else if(net->hidden_activation_function == &relu2) net->hidden_activation_derivative = &relu2_derivative;
}

void init_pretrained_ann(ANN *net){
	fill_zeros(net->dedw, net->n_weights);

	if(net->output_activation_function == &relu) net->output_activation_derivative = &relu_derivative;
	else if(net->output_activation_function == &relu2) net->output_activation_derivative = &relu2_derivative;

	if(net->hidden_activation_function == &relu) net->hidden_activation_derivative = &relu_derivative;
	else if(net->hidden_activation_function == &relu2) net->hidden_activation_derivative = &relu2_derivative;
}

//-----Utility-----
void fill_zeros(float *v, unsigned int size){
	int i;
	for(i = 0; i < size; i++){ v[i] = 0.0; }
}
void fill_number(float *v, unsigned int size, float number){
	int i;
	for(i = 0; i < size; i++){ v[i] = number; }
}

//-----Activation Functions-----
float relu(float x){
	if(x < 0.0) return 0.0;
	else if(x > 1.0) return 0.1*x+0.93;
	return x;
}

//Similar to Tanh
float relu2(float x){
	if(x < -1.0)     return 0.1*x-0.93;
	else if(x > 1.0) return 0.1*x+0.93;
	return x;
}

//-----Derivative Functions-----
float relu_derivative(float x){
	if(x < 0.0) return 0.0;
	else if(x > 1.0) return 0.1;
	return 1.0;
}
float relu2_derivative(float x){
	if(x < -1.0) return 0.1;
	else if(x > 1.0) return 0.1;
	return 1.0;
}

//----Wrapper Functions-----

void set_model_memory(ANN *model, float *weights, float *dedw, float *bias, float *output){
	model->weights = weights;
	model->dedw = dedw;
	model->bias = bias;
	model->output = output;
}

void set_model_parameters(ANN *model, unsigned int *topology, unsigned int nlayers, char activation_function){
	model->topology = topology;
	model->n_layers = nlayers;

	int i;
	int nweights = 0, nbias = 0;
	for(i = 1; i < nlayers; i++){
		nweights += topology[i]*topology[i-1];
		nbias += topology[i-1];
	}

	model->n_weights = nweights;
	model->n_bias = nbias;

	switch(activation_function){
	case 'r':
		model->output_activation_function = &relu;
		model->hidden_activation_function = &relu;
		break;
	case 'R':
		model->output_activation_function = &relu2;
		model->hidden_activation_function = &relu2;
		break;
	default:
		model->output_activation_function = &relu;
		model->hidden_activation_function = &relu;
		break;
	}
}

void set_model_hyperparameters(ANN *model, float learning_rate, float bias_learning_rate, float momentum_factor){
	model->eta = learning_rate;
	model->beta = bias_learning_rate;
	model->alpha = momentum_factor;
}

void set_learning_rate(ANN *model, float eta){
	model->eta = eta;
}

void set_bias_learning_rate(ANN *model, float beta){
	model->beta = beta;
}

void set_momentum_factor(ANN *model, float alpha){
	model->alpha = alpha;
}

void set_output_actfunc(ANN *model, char func){
	switch(func){
	case 'r':
		model->output_activation_function = &relu;
		model->output_activation_derivative = &relu_derivative;
		break;
	case 'R':
		model->output_activation_function = &relu2;
		model->output_activation_derivative = &relu2_derivative;
		break;
	default:
		model->output_activation_function = &relu;
		model->output_activation_derivative = &relu_derivative;
		break;
	}
}

void set_hidden_actfunc(ANN *model, char func){
	switch(func){
	case 'r':
		model->hidden_activation_function = &relu;
		model->hidden_activation_derivative = &relu_derivative;
		break;
	case 'R':
		model->hidden_activation_function = &relu2;
		model->hidden_activation_derivative = &relu2_derivative;
		break;
	default:
		model->hidden_activation_function = &relu;
		model->hidden_activation_derivative = &relu_derivative;
		break;
	}
}
