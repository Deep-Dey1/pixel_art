#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

const int TERM_COLS = 107;
const int TERM_ROWS = 32;

inline float gammaExpand(float c){ return pow(c/255.0f,2.2f);}
inline int gammaCompress(float c){ return int(pow(max(0.f,min(1.f,c)),1.0f/2.2f)*255);}
inline float clampf(float x,float a,float b){ return max(a,min(b,x));}

void printHalfBlock(int r1,int g1,int b1,int r2,int g2,int b2){
    cout << "\x1b[38;2;" << r1 << ";" << g1 << ";" << b1 << "m";
    cout << "\x1b[48;2;" << r2 << ";" << g2 << ";" << b2 << "m";
    cout << "▀";
    cout << "\x1b[0m";
}

int main(){
    string path;
    cout << "Enter image path: ";
    cin >> path;

    int w,h,ch;
    unsigned char* img = stbi_load(path.c_str(),&w,&h,&ch,3);
    if(!img){ cout << "Failed to load image.\n"; return 0;}

    // =============================
    // Compute scale to fit terminal
    // =============================
    int maxOutRows = TERM_ROWS-2;
    int maxImageRows = maxOutRows*2;
    float scaleX = (float)w/TERM_COLS;
    float scaleY = (float)h/maxImageRows;
    float scale = max(scaleX,scaleY);

    int targetW = int(w/scale);
    int targetH = int(h/scale);
    int outRows = targetH/2;
    int leftPad = (TERM_COLS - targetW)/2;
    if(leftPad<0) leftPad=0;

    // =============================
    // Gamma-correct input
    // =============================
    vector<float> R(w*h),G(w*h),B(w*h);
    for(int i=0;i<w*h;i++){
        R[i]=gammaExpand(img[i*3+0]);
        G[i]=gammaExpand(img[i*3+1]);
        B[i]=gammaExpand(img[i*3+2]);
    }
    stbi_image_free(img);

    // =============================
    // Render with super-sampling
    // =============================
    cout << "\nRendering High-Quality Pixel Art...\n\n";

    // Number of source pixels per terminal pixel (higher = smoother)
    int SS = max(1,(int)scale); // supersampling factor

    for(int y=0;y<outRows;y++){
        cout << string(leftPad,' ');
        for(int x=0;x<targetW;x++){
            // Top and bottom halves
            float sumR_top=0,sumG_top=0,sumB_top=0;
            float sumR_bot=0,sumG_bot=0,sumB_bot=0;
            int count_top=0,count_bot=0;

            int y_top_start = int(y*2*scale);
            int y_bot_start = int((y*2+1)*scale);
            int x_start = int(x*scale);
            int x_end = int((x+1)*scale);

            // Top half
            for(int yy=y_top_start; yy<y_top_start+SS; yy++){
                if(yy>=h) continue;
                for(int xx=x_start; xx<x_end; xx++){
                    if(xx>=w) continue;
                    int idx = yy*w + xx;
                    sumR_top += R[idx]; sumG_top += G[idx]; sumB_top += B[idx];
                    count_top++;
                }
            }
            // Bottom half
            for(int yy=y_bot_start; yy<y_bot_start+SS; yy++){
                if(yy>=h) continue;
                for(int xx=x_start; xx<x_end; xx++){
                    if(xx>=w) continue;
                    int idx = yy*w + xx;
                    sumR_bot += R[idx]; sumG_bot += G[idx]; sumB_bot += B[idx];
                    count_bot++;
                }
            }

            int r_top = gammaCompress(sumR_top/max(1,count_top));
            int g_top = gammaCompress(sumG_top/max(1,count_top));
            int b_top = gammaCompress(sumB_top/max(1,count_top));

            int r_bot = gammaCompress(sumR_bot/max(1,count_bot));
            int g_bot = gammaCompress(sumG_bot/max(1,count_bot));
            int b_bot = gammaCompress(sumB_bot/max(1,count_bot));

            printHalfBlock(r_top,g_top,b_top,r_bot,g_bot,b_bot);
        }
        cout << "\n";
    }

    return 0;
}
