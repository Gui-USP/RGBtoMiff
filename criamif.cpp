#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

// pega a distancia entre a cor de 4 bits (k) e a rgb normal (u)
int getdif(int k, Vec3b u){
    if(k==7)
        return (192 - u[2])*(192 - u[2]) + (192 - u[1])*(192 - u[1]) + (192 - u[0])*(192 - u[0]);
    if(k==8)
        return (128 - u[2])*(128 - u[2]) + (128 - u[1])*(128 - u[1]) + (128 - u[0])*(128 - u[0]);
    int mult = (k>>3)&1 ? 255 : 128;
    return ((k&1)*mult - u[2])*((k&1)*mult - u[2]) + (((k>>1)&1)*mult - u[1])*(((k>>1)&1)*mult - u[1]) + (((k>>2)&1)*mult - u[0])*(((k>>2)&1)*mult - u[0]);
}

int main(int argc, char* argv[]){

    // checagem se ta tudo certinho
    string inname;
    string outname = "result.png";
    if(argc < 2){
        cout<<"nenhum nome passado para a imagem a ser convertida, escreva agr: ";
        cin>>inname;
    }else
        inname = argv[1];
    Mat ini = imread(inname);

    if(ini.cols<320 || ini.rows < 256)
        cout<<"imagem muito pequena, tamanho minimo: 256x320\n";
    if(ini.cols>320 || ini.rows > 256)
        cout<<"imagem muito grande, cortando, tamanho ideal 256x320\n";
    Mat m(ini, Rect(0,0,320,256));
    if (!m.data)
        return cout<<"n tem imagem com o nome '"<<inname<<"', saindo"<<endl, 0;
    if(argc <3)
        cout<<"nenhum nome passado para a imagem final, ";
    else
        outname = argv[2];
    cout<<"jogando o resultado na imagem '"<<outname<<"'"<<endl;
    
    cout<<"{"<<m.rows<<", "<<m.cols<<"}"<<endl;

    // cabeçalho do .mif
    FILE* f1 = fopen("video_res1.mif","w+");
    fprintf(f1,"WIDTH=4;\nDEPTH=65536;\n\nADDRESS_RADIX=UNS;\nDATA_RADIX=UNS;\n\nCONTENT BEGIN\n");
    FILE* f2 = fopen("video_res2.mif","w+");
    fprintf(f2,"WIDTH=4;\nDEPTH=16384;\n\nADDRESS_RADIX=UNS;\nDATA_RADIX=UNS;\n\nCONTENT BEGIN\n");
    
    // percorre a imagem toda
    int i,j,best,bestdif,dif,mult;
    for(int pos=0; pos<320*256; pos++){

        // pega a posisão x e y da imagem
        i=pos%m.cols;
        j=pos/m.cols;

        // seleciona a cor mais próxima por distancia de cores
        best=0;
        bestdif = getdif(0,m.at<Vec3b>(j,i));
        for(int k=1;k<16;k++){
            dif = getdif(k,m.at<Vec3b>(j,i));
            if(dif < bestdif){
                best = k;
                bestdif = dif;
            }
        }

        // escreve no mif
        if(pos<65536)
            fprintf(f1,"\t%d : %d;\n",pos,best);
        else
            fprintf(f2,"\t%d : %d;\n",pos-65536,best);

        // gera a mesma imagem que o mif vai exibir na placa
        if(best==7)
            m.at<Vec3b>(j,i) = (Vec3b){192, 192, 192};
        else if(best==8)
            m.at<Vec3b>(j,i) = (Vec3b){128, 128, 128};
        else{
            mult = (best>>3)&1 ? 255 : 128;
            m.at<Vec3b>(j,i) = (Vec3b){ ((best>>2)&1)*mult, ((best>>1)&1)*mult, (best&1)*mult};
        }
    }
    fprintf(f1,"END;");
    fclose(f1);
    fprintf(f2,"END;");
    fclose(f2);
    imwrite(outname,m);
}
