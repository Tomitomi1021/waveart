#include<stdio.h>
#include<stdlib.h>
#include<SDL2/SDL.h>
#include<math.h>

#define SKIPCNT		(10)
#define RANGE_MIN	(-1000)
#define RANGE_MAX	(1000)
#define SCREEN_WIDTH	(640)
#define SCREEN_HEIGHT	(480)
#define FIELD_WIDTH	(640)
#define FIELD_HEIGHT	(480)

typedef struct {
	int width;
	int height;
	double* S;	//Speed to wave propergate
	double*	CF;	//Current Field
	double* PF;	//Previous Field
	double* NF;	//Next Field
	double dx;
	double dy;
	double dt;
} Field;

typedef struct {
	SDL_Point *Pixel[256];
	int PixelCnt[256];
}Pixels;

Field* mkField(int Width,int Height,double dx,double dy,double dt,double s){
	Field* f=(Field*)malloc(sizeof(Field));
	if(f==0)return 0;

	f->S=(double*)malloc(sizeof(double)*Width*Height);
	if(f->S==0){free(f);return 0;}

	f->CF=(double*)malloc(sizeof(double)*Width*Height);
	if(f->CF==0){free(f->S);free(f);return 0;}

	f->PF=(double*)malloc(sizeof(double)*Width*Height);
	if(f->PF==0){free(f->CF);free(f->S);free(f);return 0;}

	f->NF=(double*)malloc(sizeof(double)*Width*Height);
	if(f->NF==0){free(f->PF);free(f->CF);free(f->S);free(f);return 0;}
	
	f->width=Width;
	f->height=Height;
	f->dx=dx;
	f->dy=dy;
	f->dt=dt;

	for(int i=0;i<Width*Height;i++){
		f->S[i]=s;
		f->PF[i]=0;
		f->CF[i]=0;
		f->NF[i]=0;
	}

	return f;
}

void freeField(Field* f){
	free(f->S);
	free(f->PF);
	free(f->CF);
	free(f->NF);
	free(f);
}

void Field_calculate(Field* f){
	double dx=f->dx;
	double dy=f->dy;
	double dt=f->dt;

	for(int y=0;y<f->height;y++){
		for(int x=0;x<f->width;x++){
			/*
				n
			      w c e
				s
			*/
			double Cc;
			double Cn;
			double Ce;
			double Cw;
			double Cs;
			double Pc;
			double Nc;
			double s;
			Cc=f->CF[x+y*f->width];
			Pc=f->PF[x+y*f->width];
			s=f->S[x+y*f->width];
			if(0<=y-1){
				Cn=f->CF[x+(y-1)*f->width];
			}else{
				Cn=0;
			}
			if(y+1<f->height){
				Cs=f->CF[x+(y+1)*f->width];
			}else{
				Cs=0;
			}
			if(x+1<f->width){
				Ce=f->CF[(x+1)+y*f->width];
			}else{
				Ce=0;
			}
			if(0<=x-1){
				Cw=f->CF[(x-1)+y*f->width];
			}else{
				Cw=0;
			}
			Nc=2*Cc-Pc+pow(dt,2)*pow(s,2)*((Cw-2*Cc+Ce)/dx+(Cs-2*Cc+Cn)/dy);
			f->NF[x+y*f->width]=Nc;
		}
	}
	double* tmp;
	tmp=f->PF;
	f->PF=f->CF;
	f->CF=f->NF;
	f->NF=tmp;
}

void Field_transPixels(Field* f,Pixels* ps,double min,double max){
	for(int i=0;i<256;i++){
		ps->PixelCnt[i]=0;
	}
	for(int y=0;y<f->height;y++){
		for(int x=0;x<f->width;x++){
			int c=(int)((f->CF[x+y*f->width]-min)*255.0/(max-min));
			if(c>255)c=255;
			if(c<0)c=0;
			ps->Pixel[c][ps->PixelCnt[c]]=(SDL_Point){x,y};
			ps->PixelCnt[c]+=1;
		}
	}
}

void Field_Frame(Field* f){
	static int t=0;
	f->CF[320+240*f->width]=1000*sin(t*M_PI*2/100);
	t++;
}

Pixels* mkPixels(int bufsize){
	Pixels* ps=(Pixels*)malloc(sizeof(Pixels));
	if(ps==0)return 0;
	for(int i=0;i<256;i++){
		ps->Pixel[i]=(SDL_Point*)malloc(sizeof(SDL_Point)*bufsize);
		if(ps->Pixel[i]==0){
			for(int k=0;k<i;k++){
				free(ps->Pixel[k]);
			}
			free(ps);
			return 0;
		}
	}
	return ps;
}

void freePixels(Pixels* ps){
	for(int i=0;i<256;i++){
		free(ps->Pixel[i]);
	}
	free(ps);
}

int  WinMain(){
	SDL_Window* w;
	SDL_Renderer* r;
	SDL_Renderer* sr;
	SDL_Surface* S;
	SDL_Texture* T;
	SDL_Event ev;
	Pixels* ps;
	Field* f;

	f=mkField(FIELD_WIDTH,FIELD_HEIGHT,1.0/3,1.0/3,0.1,3);
	ps=mkPixels(FIELD_WIDTH*FIELD_HEIGHT);
	SDL_Init(SDL_INIT_EVERYTHING);
	w=SDL_CreateWindow("Wave",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_OPENGL);
	r=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED);
	S=SDL_CreateRGBSurface(0,FIELD_WIDTH,FIELD_HEIGHT,32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);
	sr=SDL_CreateSoftwareRenderer(S);

	int t=0;
	
	while(1){
		for(int i=0;i<SKIPCNT;i++){
			f->CF[320+240*640]+=1000*sin(t*M_PI*2.0/10.0);
			Field_Frame(f);
			Field_calculate(f);
			t++;
		}
		Field_transPixels(f,ps,RANGE_MIN,RANGE_MAX);
		for(int i=0;i<256;i++){
			SDL_SetRenderDrawColor(sr,i,i,i,255);
			SDL_RenderDrawPoints(sr,ps->Pixel[i],ps->PixelCnt[i]);
		}
		T=SDL_CreateTextureFromSurface(r,S);
		SDL_RenderCopy(r,T,NULL,NULL);
		SDL_DestroyTexture(T);
		SDL_RenderPresent(r);
		while(SDL_PollEvent(&ev)){
			switch(ev.type){
			case SDL_QUIT:
				SDL_DestroyRenderer(r);
				SDL_DestroyWindow(w);
				SDL_Quit();
				exit(0);
			}
		}
	}
}
