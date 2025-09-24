#include <iostream>
#include <vector>
#include <array>

#pragma warning(disable : 4996)

#include "raylib\include\raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raylib\include\raygui.h"

#include "Serial.h"

int IMG_WIDTH = 1280;
int IMG_HEIGHT = 720;

std::string COMport = "COM4";

struct TextBox
{
	Rectangle Rec;
	std::string Name = "";
	Color color;
	char Textarray[20] = { 0 };
	bool Is_Editable = false;
	int state = 0;//0->inactive,1->Text editable,2->Color picker
};

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(IMG_WIDTH,IMG_HEIGHT,"My Serial Plotter");
	SetTargetFPS(60);

	Font MyFont = LoadFontEx("Assets/iosevka-medium.ttf",32,0,250);
	GuiSetFont(MyFont);
	GuiSetStyle(DEFAULT,TEXT_SIZE,20);

	//Color Palette
	Color Color_Palette[] = {ORANGE,WHITE,PURPLE,PINK,DARKBROWN,DARKBLUE,DARKGREEN,GREEN};

	//GraphLegend Settings
	int GL_Height = 60;
	int GL_Rootx = 10;
	int GL_Rooty = IMG_HEIGHT - 10 - GL_Height;
	int GL_NameHeight = 20;
	int GL_NameWidth = 200;
	int GL_Name_Margin = 10;
	float LatestValues[NO_OF_CHANNELS] = { 0 };

	std::array<TextBox, NO_OF_CHANNELS> GraphNames;
	for (int i = 0;i < NO_OF_CHANNELS;i++)
	{
		std::string GraphName = TextFormat("Graph %d",i+1);//default name given
		GraphNames[i] = {Rectangle(GL_Rootx+(i+1)*GL_Name_Margin+i*GL_NameWidth,GL_Rooty+GL_Name_Margin,GL_NameWidth
		,GL_NameHeight),GraphName,Color_Palette[i]};
	}

	TextBox* ActiveGraphName = &GraphNames[0];
	int ActiveGraphIndex = 0;

	//Color Picker Settings
	float CPicker_W = 200.0f;
	float CPicker_H = 200.0f;
	float CPicker_Hue_Padding = GuiGetStyle(COLORPICKER,HUEBAR_PADDING);
	float CPicker_Hue_Width = GuiGetStyle(COLORPICKER, HUEBAR_WIDTH);
	Rectangle CPicker = {0,0,0,0};
	Rectangle CPicker_HUE = { 0,0,0,0 };


	//Graph Area Settings
	int GraphArea_Margin = 50;
	int GraphArea_Width = IMG_WIDTH - 2 * GraphArea_Margin;
	int No_DataPoints = 1600;//Number of displaying data points 
	int Default_No_DataPoints = No_DataPoints;
	float Y_ScalingFactor = 15;
	float Y_Scaling = 50;
	float X_ScalingFactor = 100;

	//BaudRate Settings
	int BRs[] = {110,300,600,1200,2400,4800,9600,14400,19200,38400,56000,57600,115200,128000,256000};
	int ActiveBRIndex = 6;

	SerialInit(COMport, BRs[ActiveBRIndex]);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);

		//for Window resizing
		IMG_WIDTH = GetScreenWidth();
		IMG_HEIGHT = GetScreenHeight();

		Y_Scaling = Y_ScalingFactor * 0.01 * IMG_HEIGHT;
		No_DataPoints = Default_No_DataPoints * 0.01 * X_ScalingFactor;
		GraphArea_Width = IMG_WIDTH - 2 * GraphArea_Margin;

		//to calculate how long the loop take
		double T1 = GetTime();

		//KeyBoard Mouse Events Checking
		bool lms = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		bool rms = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
		Vector2 mp = GetMousePosition();
		bool es = IsKeyPressed(KEY_ENTER);

		//logic for enableing Graph name editing and color picker 
		if (lms || rms)
		{
			if (!CheckCollisionPointRec(mp, CPicker_HUE))
			{
				CPicker = { 0,0,0,0 };
				CPicker_HUE = { 0,0,0,0 };

				int i = -1;
				for (auto& GN : GraphNames)
				{
					GN.state = 0;
					i++;
					GN.Is_Editable = false;

					if (CheckCollisionPointRec(mp, GN.Rec))
					{
						GN.state = 1;
						ActiveGraphName = &GN;
						ActiveGraphIndex = i;

						if (rms)
						{
							//if right mouse clicked on the Name -> Open color picker
							GN.state = 2;
							GN.Is_Editable = false;
							continue;
						}

						//if left mouse clicked on the Name -> enable Graph name edit
						GN.Is_Editable = true;
					}
				}

			}
		}


		float timestep = (float)(GraphArea_Width) / (float)(No_DataPoints);

		//Parse Data from the serial port
		std::array<std::vector<double>, NO_OF_CHANNELS>& DATA = SerialParseData();

		//removing data if exceeds the limit "No_DataPoints"
		for (int i = 0;i < NO_OF_CHANNELS;i++)
		{
			if (DATA[i].size() > No_DataPoints)
			{
				int RemoveAmount = DATA[i].size() - No_DataPoints;
				DATA[i].erase(DATA[i].begin(),DATA[i].begin()+RemoveAmount);
			}
		}

		//Draw y = 0 line
		DrawLineEx(Vector2(GraphArea_Margin,IMG_HEIGHT/2),Vector2(IMG_WIDTH - GraphArea_Margin,IMG_HEIGHT/2),1.0f,WHITE);
		
		//Drawing Graphs
		for (int i = 0;i < NO_OF_CHANNELS;i++)
		{
			for (int j = 1;j < DATA[i].size();j++)
			{
				//point j-1
				float x1 = (j - 1) * timestep + GraphArea_Margin;
				float y1 = (IMG_HEIGHT / 2 - DATA[i][j - 1] * Y_Scaling);

				//point j
				float x2 = (j) * timestep + GraphArea_Margin;
				float y2 = (IMG_HEIGHT / 2 - DATA[i][j] * Y_Scaling);

				DrawLineEx(Vector2(x1,y1),Vector2(x2,y2),3.0f,Color_Palette[i]);
			}

			//for displaying the current/latest value in the legend
			if (DATA[i].size() > 0)
			{
				LatestValues[i] = DATA[i].back();
			}
		}

		//scrolling legends
		GL_Rootx += GetMouseWheelMove() * 30;
		GL_Rooty = IMG_HEIGHT - 10 - GL_Height;
		for (int i = 0;i < NO_OF_CHANNELS;i++)
		{
			GraphNames[i].Rec.x = GL_Rootx + ((i + 1) * GL_Name_Margin + i * GL_NameWidth);
			GraphNames[i].Rec.y = GL_Rooty + GL_Name_Margin;
		}

		//displaying the current/latest value in the legend
		for (int i = 0;i < NO_OF_CHANNELS;i++)
		{
			DrawTextEx(MyFont, TextFormat("%f", LatestValues[i]), Vector2{GraphNames[i].Rec.x,GraphNames[i].Rec.y
				+GL_NameHeight + 5},20,2,Color_Palette[i]);
		}

		//Drawing a border around the legends
		Rectangle GraphLegendBorder = {GL_Rootx,GL_Rooty,((NO_OF_CHANNELS+1)*10 + NO_OF_CHANNELS*GL_NameWidth),GL_Height};
		DrawRectangleLines(GraphLegendBorder.x, GraphLegendBorder.y, GraphLegendBorder.width, GraphLegendBorder.height,RED);



		//for displaying the text box field for entering name and updating GraphNames
		for (auto& GN : GraphNames)
		{
			if (GN.Is_Editable)
			{
				//displaying a text box for entering name
				GuiTextBox(GN.Rec,GN.Textarray,20,GN.Is_Editable);
			}
			else
			{
				if (GN.Textarray[0] != '\0')
				{
					GN.Name = GN.Textarray;
				}

				DrawTextEx(MyFont, GN.Name.c_str(), Vector2{GN.Rec.x,GN.Rec.y},20,2,GN.color);
			}

		}

		//displaying color picker
		if (ActiveGraphName->state == 2)
		{
			//setting color picker dimentions
			CPicker = {ActiveGraphName->Rec.x,ActiveGraphName->Rec.y -CPicker_H,CPicker_W,CPicker_H };
			//setting color picker Hue bar dimentions
			CPicker_HUE = { ActiveGraphName->Rec.x,ActiveGraphName->Rec.y - CPicker_H,CPicker_W+CPicker_Hue_Padding
				+ CPicker_Hue_Width,CPicker_H };
			//displaying the color picker
			GuiColorPicker(CPicker,"we",&(ActiveGraphName->color));
			//setting the new chosen color
			Color_Palette[ActiveGraphIndex] = ActiveGraphName->color;

		}

		//saving the name after pressing enter
		if (es)
		{
			ActiveGraphName->Name = ActiveGraphName->Textarray;
			ActiveGraphName->Is_Editable = false;
			ActiveGraphName->Textarray[0] = '\0';
		}

		//XY Scaling of the Graphs
		Y_ScalingFactor = GuiScrollBar(Rectangle(0,0,200,30),Y_ScalingFactor,1,100);
		X_ScalingFactor = GuiScrollBar(Rectangle(220, 0, 200, 30), X_ScalingFactor, 1, 200);

		T1 = GetTime() - T1;
		DrawTextEx(MyFont, TextFormat("DrawTime : %f ms", T1 * 1000), Vector2{10,30},25,2,BLUE);

		EndDrawing();
	}
	SerialClose();

	CloseWindow();

	return 0;
}