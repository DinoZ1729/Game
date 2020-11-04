#include <iostream>
#include <windows.h>
#include <cmath>
#include <conio.h>
#include <cstdlib>
#include <time.h>

# define M_PI 3.14159265358979323846

//screen dimensions
constexpr int WIDTH=950,HEIGHT=700;

//width and height of each character in pixels
constexpr int dW=8, dH=16;

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

void drawPoint(char** platno,int A,int B,char c)
{
	B=HEIGHT/dH-1-B;
	if(A<0||B<0||A>=WIDTH/dW||B>=HEIGHT/dH)	return;
	platno[B][A]=c;
}

class Particle{
	public:
		float x,y;
		float vx=0,vy=-1000;
		void update(float dt,float dx)
		{
			t+=dt;
			x+=vx*dt-dx;
			if(x<0)
			{
				x=WIDTH-static_cast<float> (rand())/(static_cast<float> (RAND_MAX/dx));
				//y=static_cast<float> (rand())/(static_cast<float> (RAND_MAX/dx));
			}
			if(x>WIDTH)
			{
				x=static_cast<float> (rand())/(static_cast<float> (RAND_MAX/dx));
				//y=rand()%HEIGHT;
			}
			y+=vy*dt;
		}
		bool alive()
		{
			return t<256;
		}
		void resurrect()
		{
			t=0;
			if(y<0)	y=HEIGHT+rand()%HEIGHT;
		}
	private:
		float t=0.0f;
};

class Terrain{
	public:
		
		Terrain(float lacunarity, float persistance,float miny=0.0f,float height=1.0f,int color_level=5)
		:lacunarity(lacunarity),persistance(persistance),miny(miny),height(height),color_level(color_level)
		{
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
		
		void Draw(char** platno,float playerPosition=0.0f)
		{
			for(int i=0;i<HEIGHT/dH;i++){
			for(int j=0;j<WIDTH/dW;j++){
				int index=j-(playerPosition-accumulator)/dW;
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
					float k=float(y)/float(y_start+1);
					float probability=pow(3*k*k-2*k*k*k,1.0f+random);	//smoothstep
					
					char palette_ground[6]=" .:oO";
					
					int color_value=(int)(probability*color_level);
					
					if(color_value%2)	(y+x)%2?drawPoint2(P[0],P[1],palette_ground[color_value/2],index):drawPoint2(P[0],P[1],palette_ground[color_value/2+1],index);
					else				drawPoint2(P[0],P[1],palette_ground[color_value/2],index);
					
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

class Entity{
	public:
		Entity(float health,float power,float x,Terrain* terrain)
		:health(health),power(power),x(x),terrain(terrain)
		{
			result=terrain->f(x);
			y=result;
		}
		
		void Draw(char** platno,float playerPosition)
		{
			int P[2];
			convert(P,playerPosition,y);
			P[1]++;
			
			drawPoint(platno,P[0],P[1],'@');
			drawPoint(platno,P[0],P[1]+1,'@');
			drawPoint(platno,P[0],P[1]+2,'@');
			drawPoint(platno,P[0],P[1]+3,'@');
			
			drawPoint(platno,P[0]-1,P[1],'@');
			drawPoint(platno,P[0]-1,P[1]+1,'@');
			drawPoint(platno,P[0]-1,P[1]+2,'@');
			drawPoint(platno,P[0]-1,P[1]+3,'@');
			
			drawPoint(platno,P[0]+1,P[1],'@');
			drawPoint(platno,P[0]+1,P[1]+1,'@');
			drawPoint(platno,P[0]+1,P[1]+2,'@');
			drawPoint(platno,P[0]+1,P[1]+3,'@');
		}
		
		//movement
		void MoveRight()
		{
			vx=agility;
		}
		
		void MoveLeft()
		{
			vx=-agility;
		}
		
		void Jump()
		{
			if(OnGround())
				vy=jump;
		}
		
		void Drop()
		{
			y=result;
		}
		
		void Update(float dt)
		{
			//gravity
			vy-=1750.0f*dt;
			
			x+=dt*vx;
			result=terrain->f(x);
			y+=dt*vy;
			if(y<result)	y=result;
			
			vx=0.0f;
		}
		
		bool OnGround()
		{
			float epsilon=0.0001f;
			return result-epsilon<=y && y<=result+epsilon;
		}
		
		float GetX()
		{
			return x;
		}
		
	private:
		float health;
		float power;
		float x,y;
		float vx=0.0f,vy=0.0f;
		float agility=500.0f;
		float range;
		float jump=800.0f;
		float result;	//keeps track of terrain value below x
		Terrain* terrain;
};

int main()
{
	srand(static_cast <unsigned> (time(0)));
	
	float horizont=HEIGHT/2;
	
	//rain declaration
	Particle rain[100];
	for(int i=0;i<100;i++)
	{
		rain[i].y=float(HEIGHT+rand()%(2*HEIGHT));
		rain[i].x=float(rand()%WIDTH);
	}
	bool rainToday=false,rainTomorrow=true;
	
	//star declaration
	int numOfStars=rand()%10+40;
	float stars[numOfStars][2];
	for(int i=0;i<numOfStars;i++)
	{
		stars[i][0]=static_cast<float> (rand())/(static_cast<float> (RAND_MAX/WIDTH));
		stars[i][1]=static_cast<float> (rand())/(static_cast<float> (RAND_MAX/(HEIGHT-horizont)))+horizont;
	}
	float starBrightness[numOfStars];
	for(int i=0;i<numOfStars;i++)	starBrightness[i]=-static_cast<float> (rand())/(static_cast<float> (RAND_MAX/16.0f));
	
	//this will be our canvas
	char** platno=new char*[HEIGHT/dH];//[HEIGHT/dH][WIDTH/dW+1];
	for(int i=0;i<HEIGHT/dH;i++)	platno[i]=new char[WIDTH/dW+1];
	for(int i=0;i<HEIGHT/dH;i++)	platno[i][WIDTH/dW]='\0';
	//platno[HEIGHT/dH-1][WIDTH/dW]='\0';
	
	//terrain
	Terrain terrain(2.0f,0.5f,0.0f,0.3f,7);
	Terrain mountains(4.0f,0.5f,horizont-0.3f*HEIGHT/2,0.3f,4);
	//mountains.setSpeed(0.05f);
	terrain.addTrees();
	
	//entities
	Entity player(1000.0f,100.0f,0.0f,&terrain);
	constexpr float playerPosition=300.0f;
	
	//time
	const float fps=300.0f;
	const float dt=1.0f/fps;
	float accumulator = 0.0f;
	float time=0.0f;
	const float day_period=30.0f;
	float day_accumulator=0.0f;
	clock_t frameStart=clock();
	
	
	
	//main loop
	while(true)
	{
		//time
		const clock_t currentTime = clock();
		accumulator+=float(currentTime-frameStart)/CLOCKS_PER_SEC;
		time+=accumulator;
		day_accumulator+=accumulator;
		
		frameStart=currentTime;
		
		if(accumulator>=0.2f)	accumulator=0.2f;
		
		//logic
		
		while(day_accumulator>=day_period)
		{
			rainToday=rainTomorrow;
			int random=rand()%100;
			
			rainTomorrow=rainToday;
			if(rainToday)
			{
				if(random<100)	rainTomorrow=false;
			}
			else
			{
				if(random<100)	rainTomorrow=true;
			}
			day_accumulator-=day_period;
		}
		
		while(accumulator>=dt)
		{
			float start=player.GetX();
			if(GetKeyState(VK_LEFT)&0x8000)		player.MoveLeft();
			if(GetKeyState(VK_RIGHT)&0x8000)	player.MoveRight();
			if(GetKeyState(VK_UP)&0x8000)		player.Jump();
			if(GetKeyState(VK_DOWN)&0x8000)		player.Drop();
			if(GetKeyState(VK_SPACE)&0x8000)	//for now space will be quit button
			{
				for(int i=0;i<HEIGHT/dH;i++)	delete[] platno[i];
				delete[] platno;
				return 0;
			}
			player.Update(dt);
			
			float finish=player.GetX();
			terrain.update(finish-start);
			//mountains.update(finish-start);
			
			for(int i=0;i<100;i++)
			{
				rain[i].update(dt,finish-start);
				if(rainToday)
					rain[i].resurrect();
			}	
			
			accumulator-=dt;
		}
		
		//drawing
		gotoxy(0,0);
		
		//SKY
		
		const float matrix[4][4]=
		{
			{ 1, 9, 3,11},
			{13, 5,15, 7},
			{ 4,12, 2,10},
			{16, 8,14, 6}
		};
		float sky_color=32*sin(time/day_period*2*M_PI);
		
		for(int i=0;i<HEIGHT/dH;i++){
		for(int j=0;j<WIDTH/dW;j++){
			!rainToday&&matrix[i%4][j%4]<=sky_color?drawPoint(platno,j,i,'.'):drawPoint(platno,j,i,' ');
		}}
		
		
		
		//sun & moon
		const char sun[6][13]=
		{
			"    @@@@    ",
			"  @@@@@@@@  ",
			" @@@@@@@@@@ ",
			" @@@@@@@@@@ ",
			"  @@@@@@@@  ",
			"    @@@@    "
		};
		const char moon[6][13]=
		{
			"      @@    ",
			"       @@@  ",
			"        @@@ ",
			"        @@@ ",
			"       @@@  ",
			"      @@    "
		};
		float
		sun_position=sky_color/48*(HEIGHT-horizont)/dH+horizont/dH-6,
		moon_position=sky_color/48*(horizont-HEIGHT)/dH+horizont/dH-6;
			
		//stars
		if(sky_color<8&&!rainToday)
		{
			for(int i=0;i<numOfStars;i++)
			{
				int P[2];
				convert(P,stars[i][0],stars[i][1]);
				if(starBrightness[i]>sky_color)	drawPoint(platno,P[0],P[1],'+');
			}
		}
		//sun
		for(int i=0;i<6;i++){
			for(int j=0;j<12;j++){
				if(sun[i][j]!=' ')drawPoint(platno,WIDTH/dW/2+6-j,sun_position+3-i,sun[i][j]);
			}
		}
		//moon
		for(int i=0;i<6;i++){
			for(int j=0;j<12;j++){
				if(moon[i][j]!=' ')drawPoint(platno,WIDTH/dW/2+6-j,moon_position+3-i,moon[i][j]);
			}
		}
		
		//mountains.Draw(platno);
		
		//sea
		for(int i=0;i<=WIDTH/dW;i++){
		for(int j=horizont/dH;j>=0;j--){
			drawPoint(platno,i,j,' ');
		}}
		
		//reflections
		
		
		//sun
		int sun_relfection=2*(int)horizont/dH-(int)sun_position;
		for(int i=0;i<6;i++){
			for(int j=0;j<12;j++){
				if(sun_relfection+i-4<horizont/dH)
					if(sun[i][j]!=' ')drawPoint(platno,WIDTH/dW/2+6-j,sun_relfection+i-4,sun[i][j]);
			}
		}
		
		//moon
		int moon_relfection=2*(int)horizont/dH-(int)moon_position;
		for(int i=0;i<6;i++){
			for(int j=0;j<12;j++){
				if(moon_relfection+i-4<horizont/dH)
					if(moon[i][j]!=' ')drawPoint(platno,WIDTH/dW/2+6-j,moon_relfection+i-4,moon[i][j]);
			}
		}
		//stars
		if(sky_color<8&&!rainToday)
		{
			for(int i=0;i<numOfStars;i++)
			{
				int P[2];
				convert(P,stars[i][0],2*horizont-stars[i][1]);
				if(starBrightness[i]>sky_color)	drawPoint(platno,P[0],P[1],'+');
			}
		}
		
		for(int i=0;i<=WIDTH/dW;i++)	drawPoint(platno,i,horizont/dH,'=');
		
		
		//rain
		if(rainToday)	for(int i=0;i<100;i++)
		{
			int P[2];
			convert(P,rain[i].x,rain[i].y);
			drawPoint(platno,P[0],P[1],'|');
		}
		int thunder_int=(int)(100*day_accumulator/day_period);
		bool thunder = rainToday && thunder_int%20 == 0 && thunder_int%40;
		
		if(thunder)
		{
			int x=rand()%(WIDTH/dW),y=HEIGHT/dH;
			
			int num_of_turns=5+rand()%3;
			
			for(int k=num_of_turns-1;k>=0;k--)
			{
				bool rigth=rand()%2;
				for(;y>=k*HEIGHT/dH/num_of_turns;y--)
				{
					drawPoint(platno,x,y,219);
					x+=2*rigth-1;
				}
			}
		}
		
		terrain.Draw(platno,playerPosition);
		player.Draw(platno,playerPosition);
		for(int i=0;i<HEIGHT/dH;i++)
		{
			puts(platno[i]);
		}
	}
	return 0;
}
