#include "SDKDLL.h"
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <math.h>

float bright_hist[8];
BYTE colors[1000][3];
COLOR_MATRIX first_point;
COLOR_MATRIX second_point;
COLOR_MATRIX color_matrix;

void select_random_points() {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < MAX_LED_COLUMN; j++) {
            first_point.KeyColor[i][j].r = second_point.KeyColor[i][j].r;
            first_point.KeyColor[i][j].g = second_point.KeyColor[i][j].g;
            first_point.KeyColor[i][j].b = second_point.KeyColor[i][j].b;
            int index = rand() % 1000;
            second_point.KeyColor[i][j].r = colors[index][0];
            second_point.KeyColor[i][j].g = colors[index][1];
            second_point.KeyColor[i][j].b = colors[index][2];
        }
    }
}

// 0 -> 3
// 1 -? 4
// 2 -> 5
// 3 -> 5
// 4 -> 4
// 5 -> 3

void fill_output_point(float progress) {
    for (int i = 0; i < 6; i++) {
        float brightness = bright_hist[i < 3 ? 3+i : 8-i];
        for (int j = 0; j < MAX_LED_COLUMN; j++) {
            color_matrix.KeyColor[i][j].r = (BYTE)(brightness*((1.0-progress)*((float)first_point.KeyColor[i][j].r) + (progress)*((float)second_point.KeyColor[i][j].r)));
            color_matrix.KeyColor[i][j].g = (BYTE)(brightness*((1.0-progress)*((float)first_point.KeyColor[i][j].g) + (progress)*((float)second_point.KeyColor[i][j].g)));
            color_matrix.KeyColor[i][j].b = (BYTE)(brightness*((1.0-progress)*((float)first_point.KeyColor[i][j].b) + (progress)*((float)second_point.KeyColor[i][j].b)));
        }
    }
}

int main() {
    srand (time(NULL));
    FILE* file = fopen("D:\\Projects\\kblights\\colors.txt", "rb");
    //FILE* f2 = fopen("D:\\Projects\\kblights\\test.csv", "w+");
    if (file == NULL) {
        printf("Couldn't read colors file\n");
        return -1;
    }
    for (int i = 0; i < 1000; i++) {
        fscanf(file, "%hhu %hhu %hhu", &colors[i][0], &colors[i][1], &colors[i][2]);
    }
    fclose(file);

    if (!IsDevicePlug(DEV_CK550_552)) {
        printf("Keyboard not connected\n");
        return -1;
    }
    SetControlDevice(DEV_CK550_552);
    EnableLedControl(true, DEV_CK550_552);

    select_random_points();
    select_random_points();

    float count = 0;
    int idx = 0;
    float samples[100];
    for (int i = 0; i < 100; i++) {
        samples[i] = 0;
    }

    while (1) {
        if (count >= 40) {
            select_random_points();
            count = 0;
        }
        float progress = ((float)count)/40.0;
        float meas = GetNowVolumePeekValue();
        samples[idx] = meas;
        idx = (idx + 1) % 100;
        float min = samples[0];
        float max = samples[0];
        float acc = 0;
        for (int i = 0; i < 100; i++) {
            if (samples[i] < min) min = samples[i];
            if (samples[i] > max) max = samples[i];
            acc += samples[i];
        }
        acc /= 100;
        float var = 0;
        for (int i = 0; i < 100; i++) {
            var += (acc-samples[i])*(acc-samples[i]);
        }
        var = sqrtf(var);
        var = var < 0.05 ? 0.05 : 0.25*var;
        float meas2 = (meas-(acc-var))/(2.f*var);
        meas2 = meas2 > 1.f ? 1.f : (meas2 < 0.f ? 0.f : meas2);
        //fprintf(f2, "%.5f,%.5f,%.5f,%.5f\n", meas, meas2, acc, var);
        //float vol = 0.3;
        //vol += 0.7*meas;
        //if (max-min > 0.001) vol += 0.4*((meas-min)/(max-min));
        //fill_output_point(progress, vol > 1 ? 1 : (vol < 0 ? 0 : vol));
        for (int i = 0; i < 5; i++) bright_hist[i] = bright_hist[i+1];
        bright_hist[5] = 0.3f+0.7f*meas2;
        fill_output_point(progress);
        SetAllLedColor(color_matrix, DEV_CK550_552);
        Sleep(10);
        //count += vol < 0.8 ? 1 : (1 + (vol-0.8)*3);
        count += 1 + 8*powf(meas2, 6);
    }

    Sleep(1000);
    EnableLedControl(false, DEV_CK550_552);
    SwitchLedEffect(EFF_MULTI_1, DEV_CK550_552);
    //fclose(f2);

    return 0;
}