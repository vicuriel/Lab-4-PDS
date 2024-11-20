#include <project.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PI 3.14159265358979323846
#define N_Rx_float 50

#if defined (__GNUC__)
    asm(".global _printf_float");
#endif
static uint8 n = 0;
static uint8 Rx_int8[4*N_Rx_float];
static float Rx_float[N_Rx_float];
static float Tx_float[2*N_Rx_float];

void calculateDFT(float *x, float *dft_x, uint8 len_x) {
   uint8 n = len_x;
  // Calculate DFT coefficients
  for (uint8 k = 0; k < n; k++) {
    int real_index = 2 * k;
    int imag_index = 2 * k + 1;

    dft_x[real_index] = 0;
    dft_x[imag_index] = 0;

    for (uint8 m = 0; m < n; m++) {
      dft_x[real_index] += x[m] * cos(2 * M_PI * k * m / n);
      dft_x[imag_index] -= x[m] * sin(2 * M_PI * k * m / n);
    }
  }
}

void convertToIntFloat(const uint8_t* input, float* output, size_t inputSize) {
    // Asegúrate de que inputSize sea un múltiplo de 4
    for (size_t i = 0; i < inputSize; i += 4) {
        // Asumiendo little endian; ajusta si es necesario
        uint32_t temp = input[i] | (input[i + 1] << 8) | (input[i + 2] << 16) | (input[i + 3] << 24);
        memcpy(output + (i / 4), &temp, sizeof(float));
    }
}


CY_ISR_PROTO(UART_handler) {
   
    Rx_int8[n++] = UART_ReadRxData();
    if (n == 4*N_Rx_float) {
        Led_Write(~Led_Read());
        convertToIntFloat(Rx_int8,Rx_float,N_Rx_float*4);
        calculateDFT(Rx_float,Tx_float,N_Rx_float);
        for(size_t i = 0; i < N_Rx_float*2; i++) Chart_1_Plot(Tx_float[i]);      
        n = 0;
    }
    isr_UART_ClearPending();
}

int main(void) {
    UART_Start();
    isr_UART_StartEx(UART_handler);
    isr_UART_Enable();
    CyGlobalIntEnable;
    for (;;) {}
}

