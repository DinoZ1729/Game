#include <iostream>
#include <windows.h>
#include <cmath>
#include <conio.h>
#include <cstdlib>
#include <time.h>

using namespace std;

//screen dimensions

#define WIDTH 800
#define HEIGHT 600

//width and height of each character in pixels
const int dW=8, dH=16;

//set cursor at the start to avoid flickering
void gotoxy(short x, short y)
{
	COORD coord = {x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

//convert from world to screen coordinates
void convert(int P[2], float x, float y)
{
	P[0]=(int)x/dW;
	P[1]=(int)y/dH;
}
void convert(int P[2], int x, int y)
{
	P[0]=x/dW;
	P[1]=y/dH;
}

void drawPoint(char platno[HEIGHT/dH][WIDTH/dW+1],int A,int B,char c)
{
	B=HEIGHT/dH-1-B;
	if(A<0||B<0||A>=WIDTH/dW||B>=HEIGHT/dH)	return;
	platno[B][A]=c;
}

class Terrain{
	public:
		
		Terrain(float lacunarity, float persistance,float miny=0.0f,float height=1.0f,int color_level=5)
		{
			this->lacunarity=lacunarity;
			this->persistance=persistance;
			this->miny=miny;
			this->height=height;
			this->color_level=color_level;
			chunk[0]=new char*[HEIGHT/dH];
			for(int i=0;i<HEIGHT/dH;i++)
				chunk[0][i]=new char[WIDTH/dW];
			
			chunk[1]=new char*[HEIGHT/dH];
			for(int i=0;i<HEIGHT/dH;i++)
				chunk[1][i]=new char[WIDTH/dW];
			
			chunk[2]=new char*[HEIGHT/dH];
			for(int i=0;i<HEIGHT/dH;i++)
				chunk[2][i]=new char[WIDTH/dW];
			
			for(int i=0;i<10;i++)
				offset[i]=static_cast<float> (rand())/(static_cast<float> (RAND_MAX/100.0f));
				
			calculate(0);
			calculate(1);
			calculate(2);
		}
		
		//function of terrain
		float f(float x)
		{
			return height*HEIGHT*(noise(x/WIDTH)+1.0f)/2.0f+miny;
		}
		
		void Draw(char platno[HEIGHT/dH][WIDTH/dW+1])
		{
			for(int i=0;i<HEIGHT/dH;i++){
			for(int j=0;j<WIDTH/dW;j++){
				int index=j+accumulator/dW;
				char c;
				if(index<0)		c=chunk[0][i][index+WIDTH/dW];
				else if(index>=WIDTH/dW)c=chunk[2][i][index-WIDTH/dW];
				else			c=chunk[1][i][index];
				if(c!='W')	platno[i][j]=c;
			}}
		}
		
		void update(float dx)
		{
			accumulator+=dx*speed;
			while(accumulator>=float(WIDTH))
			{
				accumulator-=WIDTH;
				cameraPosition+=WIDTH;
				
				for(int i=0;i<HEIGHT/dH;i++){
				for(int j=0;j<WIDTH/dW;j++){
					chunk[0][i][j]=chunk[1][i][j];
					chunk[1][i][j]=chunk[2][i][j];
				}}
				
				calculate(2);
			}
			
			while(accumulator<0.0f)
			{
				accumulator+=WIDTH;
				cameraPosition-=WIDTH;
				
				for(int i=0;i<HEIGHT/dH;i++){
				for(int j=0;j<WIDTH/dW;j++){
					chunk[2][i][j]=chunk[1][i][j];
					chunk[1][i][j]=chunk[0][i][j];
				}}
				
				calculate(0);
			}
		}
		
		void calculate(int index)
		{
			for(int i=0;i<HEIGHT/dH;i++){
			for(int j=0;j<WIDTH/dW;j++){
				chunk[index][i][j]='W';
			}}
			for(int x=0;x<WIDTH/dW;x++){
				int P[2];
				int y_start=f(x*dW+cameraPosition+index*WIDTH)/dH;
				float random=static_cast<float> (rand())/(static_cast<float>(RAND_MAX/0.2f));
				for(int y=y_start;y>=0;y--){
					convert(P,x*dW,y*dH);
					float k=float(y)/float(y_start);
					float probability=pow(3*k*k-2*k*k*k,1.0f+random);	//smoothstep
					
					char palette_ground[6]=" .:oO";
					
					int color_value=(int)(probability*color_level);
					
					if(color_value%2)	(y+x)%2?drawPoint2(P[0],P[1],palette_ground[color_value/2],index):drawPoint2(P[0],P[1],palette_ground[color_value/2+1],index);
					else				drawPoint2(P[0],P[1],palette_ground[color_value/2],index);
					
					/*
					if(0.8f<probability)		drawPoint2(P[0],P[1],':',index);
					else if(0.6f<probability)	(y+x)%2?drawPoint2(P[0],P[1],':',index):drawPoint2(P[0],P[1],'.',index);
					else if(0.4f<probability)	drawPoint2(P[0],P[1],'.',index);
					else if(0.2f<probability)	(y+x)%2?drawPoint2(P[0],P[1],'.',index):drawPoint2(P[0],P[1],' ',index);
					else						drawPoint2(P[0],P[1],' ',index);
					*/
				}
				convert(P,x*dW,y_start*dH);
				drawPoint2(P[0],P[1],'O',index);
			}
			
			//trees
			if(trees)
			{
				char trees[3][7][11]=
				{
					{
						"          ",
						"   ####   ",
						"  ########",
						" #########",
						"  ##||### ",
						"    ||    ",
						"    ||    "
					},
					{
						"    #     ",
						"  #####   ",
						" #######  ",
						" ##\\#//## ",	// \ is a special char
						" ###||### ",
						"    ||    ",
						"    ||    "
					},
					{
						"    ##    ",
						"   ####   ",
						"  ######  ",
						" ######## ",
						"##########",
						"    ||    ",
						"    ||    "
					}
					
				};
				
				int num_of_trees=rand()%3+1;
				int position[num_of_trees];
				int type[num_of_trees];
				
				for(int i=0;i<num_of_trees;i++)	type[i]=rand()%3;
				
				for(int i=0;i<num_of_trees;i++)
				{
					while(true)
					{
						position[i]=rand()%(WIDTH/dW);
						if(position[i]<=5||WIDTH/dW-position[i]<=5)	continue;
						break;
					}
				}
				for(int i=0;i<num_of_trees;i++){
					int y_start=f(position[i]*dW+cameraPosition+index*WIDTH)/dH+1;
					for(int x=-5;x<5;x++)
					{
						for(int y=0;y<7;y++)
						{
						
						if(trees[type[i]][6-y][x+5]!=' ')
							drawPoint2(x+position[i],y+y_start,trees[type[i]][6-y][x+5],index);
						}
					}
				}
				
			}
			
			
		}
		
		void drawPoint2(int A,int B,char c,int i)
		{
			B=HEIGHT/dH-1-B;
			if(A<0||B<0||A>=WIDTH/dW||B>=HEIGHT/dH)	return;
			chunk[i][B][A]=c;
		}
		
		void setColorLevel(int k)
		{
			color_level=k;
		}
		
		void setSpeed(float speed)
		{
			this->speed=speed;
		}
		
		void addTrees()
		{
			trees=true;
		}
		
		void removeTrees()
		{
			trees=false;
		}
		
		~Terrain()
		{
			for(int i=0;i<HEIGHT/dH;i++)
				delete[] chunk[0][i];
			delete[] chunk[0];
			
			for(int i=0;i<HEIGHT/dH;i++)
				delete[] chunk[1][i];
			delete[] chunk[1];
			
			for(int i=0;i<HEIGHT/dH;i++)
				delete[] chunk[2][i];
			delete[] chunk[2];
		}
	private:
		//returns value from -1.0 to 1.0
		float noise(float x)
		{
			float sum=0.0f;
			float amplitude=1.0f;
			float frequency=1.0f;
			for(int i=0;i<10;i++)
			{
				sum+=amplitude*sin(frequency*x+offset[i]);
				amplitude*=persistance;
				frequency*=lacunarity;
			}
			return sum*(1-persistance);//geometric sum
		}
		//function
		float offset[10];
		float lacunarity;
		float persistance;
		float miny;
		float height;
		//drawing
		char** chunk[3];
		float accumulator=0.0f;
		float cameraPosition=-WIDTH;
		float speed=1.0f;
		
		bool trees=false;
		int color_level=5;
};


int main()
{
	srand(static_cast <unsigned> (time(0)));
	
	//this will be our canvas
	char platno[HEIGHT/dH][WIDTH/dW+1];
	for(int i=0;i<HEIGHT/dH;i++)	platno[i][WIDTH/dW]='\n';
	platno[HEIGHT/dH-1][WIDTH/dW]='\0';
	
	Terrain terrain(2.0f,0.5f,0.0f,0.5f,7);
	Terrain mountains(4.0f,0.5f,200.0f,0.67f,4);
	mountains.setSpeed(0.05f);
	terrain.addTrees();
	
	const float fps=15.0f;
	const float dt=1.0f/fps;
	float accumulator = 0.0f;
	
	clock_t frameStart=clock();
	
	float offset=0.0f;
	
	//main loop
	while(true)
	{
		//time
		const clock_t currentTime = clock();
		accumulator+=float(currentTime-frameStart)/CLOCKS_PER_SEC;
		frameStart=currentTime;
		
		if(accumulator>=0.2f)	accumulator=0.2f;
		
		
		
		while(accumulator>=dt)
		{
			//logic
			float start=offset;
			if(GetKeyState(VK_LEFT)&0x8000)		offset-=500*dt;
			if(GetKeyState(VK_RIGHT)&0x8000)	offset+=500*dt;
			float finish=offset;
			terrain.update(finish-start);
			mountains.update(finish-start);
			accumulator-=dt;
		}
		
		//drawing
		gotoxy(0,0);
		for(int i=0;i<HEIGHT/dH;i++){
		for(int j=0;j<WIDTH/dW;j++){
			(i+j)%2?platno[i][j]='.':platno[i][j]=' ';
		}}
		mountains.Draw(platno);
		//sea
		for(int i=0;i<=WIDTH/dW;i++){
		for(int j=HEIGHT/dH/2;j>=0;j--){
			drawPoint(platno,i,j,' ');
		}}
		for(int i=0;i<=WIDTH/dW;i++)	drawPoint(platno,i,HEIGHT/dH/2,'=');;
		
		terrain.Draw(platno);
		puts(platno[0]);
	}
	return 0;
}
