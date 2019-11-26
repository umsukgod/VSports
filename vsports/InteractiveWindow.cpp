#include "InteractiveWindow.h"
#include "../render/GLfunctionsDART.h"
#include "../model/SkelMaker.h"
#include "../model/SkelHelper.h"
#include "../pyvs/EnvironmentPython.h"
#include "./common/loadShader.h"
// #include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
using namespace glm;
#include <iostream>
using namespace dart::dynamics;
using namespace dart::simulation;
using namespace std;

namespace p = boost::python;
namespace np = boost::python::numpy;
enum key_state {NOTPUSHED, PUSHED} keyarr[127];

std::chrono::time_point<std::chrono::system_clock> time_check_s = std::chrono::system_clock::now();

void time_check_start()
{
	time_check_s = std::chrono::system_clock::now();
}

void time_check_end()
{
	std::chrono::duration<double> elapsed_seconds;
	elapsed_seconds = std::chrono::system_clock::now()-time_check_s;
	std::cout<<elapsed_seconds.count()<<std::endl;
}

double floorDepth = -0.1;

void
IntWindow::
initWindow(int _w, int _h, char* _name)
{
	mWindows.push_back(this);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE | GLUT_ACCUM);
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(_w, _h);
	mWinIDs.push_back(glutCreateWindow(_name));
	// glutHideWindow();
	glutDisplayFunc(displayEvent);
	glutReshapeFunc(reshapeEvent);
	glutKeyboardFunc(keyboardEvent);
	glutKeyboardUpFunc(keyboardUpEvent);
	glutMouseFunc(mouseEvent);
	glutMotionFunc(motionEvent);
	glutTimerFunc(mDisplayTimeout, timerEvent, 0);
	mScreenshotTemp.resize(4*_w*_h);
	mScreenshotTemp2.resize(4*_w*_h);

}



IntWindow::
IntWindow()
:SimWindow(), mIsNNLoaded(false)
{
	mEnv = new Environment(30, 600, 2);
	initCustomView();
	initGoalpost();

	mm = p::import("__main__");
	mns = mm.attr("__dict__");
	sys_module = p::import("sys");

	boost::python::str module_dir = "../pyvs";
	sys_module.attr("path").attr("insert")(1, module_dir);
	// p::exec("import os",mns);
	// p::exec("import sys",mns);
	// p::exec("import math",mns);
	// p::exec("import sys",mns);
	p::exec("import torch",mns);
	p::exec("import torch.nn as nn",mns);
	p::exec("import torch.optim as optim",mns);
	p::exec("import torch.nn.functional as F",mns);
	p::exec("import torchvision.transforms as T",mns);
	p::exec("import numpy as np",mns);
	p::exec("from Model import *",mns);
	controlOn = false;
	mActions.resize(mEnv->mNumChars);
	for(int i=0;i<mActions.size();i++)
	{
		mActions[i] = Eigen::Vector4d(0.0, 0.0, 0.0, 0.0);
	}
// GLenum err = glewInit();
// if (err != GLEW_OK)
//   cout<<"Not ok with glew"<<endl; // or handle the error in a nicer way
// if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
//   cout<<"Not ok with glew version"<<endl; // or handle the error in a nicer way
//   cout<< glewGetString(err) <<endl; // or handle the error in a nicer way

}

IntWindow::
IntWindow(const std::string& nn_path)
:IntWindow()
{
	mIsNNLoaded = true;


	p::str str = ("num_state = "+std::to_string(mEnv->getNumState())).c_str();
	p::exec(str,mns);
	str = ("num_action = "+std::to_string(mEnv->getNumAction())).c_str();
	p::exec(str, mns);

	nn_sc_module = new boost::python::object[mEnv->mNumChars];
	p::object *sc_load = new p::object[mEnv->mNumChars];
	reset_sc_hidden = new boost::python::object[mEnv->mNumChars];

	nn_la_module = new boost::python::object[mEnv->mNumChars];
	p::object *la_load = new p::object[mEnv->mNumChars];
	reset_la_hidden = new boost::python::object[mEnv->mNumChars];

	for(int i=0;i<mEnv->mNumChars;i++)
	{
		nn_sc_module[i] = p::eval("SchedulerNN(num_state, num_action).cuda()", mns);
		sc_load[i] = nn_sc_module[i].attr("load");
		reset_sc_hidden[i] = nn_sc_module[i].attr("reset_hidden");
		sc_load[i](nn_path+".pt");
		// if(i== 0|| i==1 || true)
		// 	sc_load[i](nn_path+".pt");
		// else
		// 	sc_load[i]("../save/goalReward/max.pt");
	}
	// cout<<"3344444444"<<endl;
	// mActions.resize(mEnv->mNumChars);
	// for(int i=0;i<mActions.size();i++)
	// {
	// 	mActions[i] = Eigen::Vector4d(0.0, 0.0, 0.0, 0.0);
	// }
}

void
IntWindow::
initialize()
{
	// glewExperimental = GL_TRUE; 
	// glewInit();
	// GLuint VertexArrayID;
	// glGenVertexArrays(1, &VertexArrayID);
	// glBindVertexArray(VertexArrayID);

	// programID = loadShaders( "../vsports/shader/IntVertexShader.vertexshader", "../vsports/shader/IntVertexShader.fragmentshader" );
	// GLfloat width = 4;
	// GLfloat height = 3;
	// static const GLfloat g_vertex_buffer_data[] = {
	//   width/12.0f, height/12.0f, 0.0f,
	//    -width/12.0f, height/12.0f, 0.0f,
	//    -width/12.0f,  -height/12.0f, 0.0f,
	//    // width,  -height, 0.0f,
	// };
	// glUseProgram(programID);
	// // 이것이 우리의 버텍스 버퍼를 가리킵니다.
	// // GLuint vertexbuffer;
	// // 버퍼를 하나 생성합니다. vertexbuffer 에 결과 식별자를 넣습니다
	// glGenBuffers(1, &vertexbuffer);
	// // 아래의 명령어들은 우리의 "vertexbuffer" 버퍼에 대해서 다룰겁니다
	// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// // 우리의 버텍스들을 OpenGL로 넘겨줍니다
	// glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
}

void
IntWindow::
initCustomView()
{
	// mCamera->eye = Eigen::Vector3d(3.60468, -4.29576, 1.87037);
	// mCamera->lookAt = Eigen::Vector3d(-0.0936473, 0.158113, 0.293854);
	// mCamera->up = Eigen::Vector3d(-0.132372, 0.231252, 0.963847);
	mCamera->eye = Eigen::Vector3d(0.0, 0.0, 10.0);
	mCamera->lookAt = Eigen::Vector3d(0.0, 0.0, 0.0);
	mCamera->up = Eigen::Vector3d(0.0, 1.0, 0.0);

}

void
IntWindow::
initGoalpost()
{
	redGoalpostSkel = SkelHelper::makeGoalpost(Eigen::Vector3d(-4.0, 0.0, 0.25 + floorDepth), "red");
	blueGoalpostSkel = SkelHelper::makeGoalpost(Eigen::Vector3d(4.0, 0.0, 0.25 + floorDepth), "blue");

	mWorld->addSkeleton(redGoalpostSkel);
	mWorld->addSkeleton(blueGoalpostSkel);
}



void
IntWindow::
keyboard(unsigned char key, int x, int y)
{
	SkeletonPtr manualSkel = mEnv->getCharacter(0)->getSkeleton();

	switch(key)
	{
		case 'c':
			// cout<<mCamera->eye.transpose()<<endl;
			// cout<<mCamera->lookAt.transpose()<<endl;
			// cout<<mCamera->up.transpose()<<endl;
			break;
		case 'w':
			keyarr[int('w')] = PUSHED;
			break;
		case 's':
			keyarr[int('s')] = PUSHED;
			break;
		case 'a':
			keyarr[int('a')] = PUSHED;
			break;
		case 'd':
			keyarr[int('d')] = PUSHED;
			break;
		case 'g':
			keyarr[int('g')] = PUSHED;
			break;
		case 'h':
			keyarr[int('h')] = PUSHED;
			break;
		case 'r':
			mEnv->reset();
			// for(int i=0;i<2;i++){
			// 	reset_sc_hidden[i]();
			// }


			// reset_hidden[2]();
			// reset_hidden[3]();
			break;
		case 'l':
			controlOn = !controlOn;
			break;

		default: SimWindow::keyboard(key, x, y);
	}
}
void
IntWindow::
keyboardUp(unsigned char key, int x, int y)
{
	SkeletonPtr manualSkel = mEnv->getCharacter(0)->getSkeleton();

	switch(key)
	{
		case 'w':
			keyarr[int('w')] = NOTPUSHED;
			break;
		case 's':
			keyarr[int('s')] = NOTPUSHED;
			break;
		case 'a':
			keyarr[int('a')] = NOTPUSHED;
			break;
		case 'd':
			keyarr[int('d')] = NOTPUSHED;
			break;
		case 'g':
			keyarr[int('g')] = NOTPUSHED;
			break;
		case 'h':
			keyarr[int('h')] = NOTPUSHED;
			break;
	}
}
void
IntWindow::
timer(int value)
{
	if(mPlay)
		step();
	// display();
	SimWindow::timer(value);
}

void
IntWindow::
applyKeyEvent()
{
	double power = 1.0;
	if(keyarr[int('w')]==PUSHED)
	{
		mActions[0][1] += power;
	}
	if(keyarr[int('s')]==PUSHED)
	{
		mActions[0][1] += -power;
	}
	if(keyarr[int('a')]==PUSHED)
	{
		mActions[0][0] += -power;
	}
	if(keyarr[int('d')]==PUSHED)
	{
		mActions[0][0] += power;
	}
	if(keyarr[int('g')]==PUSHED)
	{
		mActions[0][3] += 0.1;
	}
	else
	{
		// mActions[0][3] -= 0.1;
		// if(mActions[0][3] < -0.1)
		// 	mActions[0][3] = -0.1;
		mActions[0][3] = -0.1;
	}
	if(keyarr[int('h')]==PUSHED)
	{
		mActions[0][3] = 1.0;
		Eigen::Vector3d curVel = mEnv->getState(0);
	}
}

void
IntWindow::
step()
{
	if(mEnv->isTerminalState())
	{
		sleep(1);
		mEnv->reset();
	}

	if(mIsNNLoaded)
		getActionFromNN(true);
	// cout<<"step in intWindow"<<endl;


	// applyKeyEvent();
	if(mEnv->mNumChars == 4)
	{
		// mActions[0] = mEnv->getActionFromBTree(0);
		mActions[1] = mEnv->getActionFromBTree(1);
		mActions[2] = mEnv->getActionFromBTree(2);
		mActions[3] = mEnv->getActionFromBTree(3);
	}
	else if(mEnv->mNumChars == 2)
	{
		// mActions[0] = mEnv->getActionFromBTree(0);
		// mActions[1] = mEnv->getActionFromBTree(1);
	}
	// cout<<mActions[1].transpose()<<endl;

	for(int i=0;i<mEnv->mNumChars;i++)
	{
		mEnv->getState(i);
		// mActions[i] = Eigen::Vector4d(0.0, 0.0, 0.0, 0.0);
		mEnv->setAction(i, mActions[i]);
	}

	mEnv->stepAtOnce();
	mEnv->getRewards();
	for(int i=0;i<mActions.size();i++)
	{
		mActions[i].segment(0,3) = Eigen::Vector3d(0.0, 0.0, 0.0);
	}
}

void
IntWindow::
getActionFromNN(bool vsHardcodedAI)
{
	p::object get_sc_action;
	p::object get_la_action;

	mActions.clear();
	mActions.resize(2);

	for(int i=0;i<mEnv->mNumChars;i++)
	{
		Eigen::VectorXd state = mEnv->getState(i);

		Eigen::VectorXd mAction(mEnv->getNumAction());
		if((vsHardcodedAI && (i == 1)))
		{

			for(int j=0;j<3;j++)
			{
				mAction[j] = 0.0;
			}

			mAction[3] = 1.0;
			mAction[3] = -1.0;
			mActions[i] = mAction;
		}
		else
		{
			get_sc_action = nn_sc_module[i].attr("get_action");

			p::tuple shape = p::make_tuple(state.size());
			np::dtype dtype = np::dtype::get_builtin<float>();
			np::ndarray state_np = np::empty(shape, dtype);

			float* dest = reinterpret_cast<float*>(state_np.get_data());
			for(int j=0;j<state.size();j++)
			{
				dest[j] = state[j];
			}

			p::object temp = get_sc_action(state_np);
			np::ndarray action_np = np::from_object(temp);
			float* srcs = reinterpret_cast<float*>(action_np.get_data());
			for(int j=0;j<mAction.rows();j++)
			{
				mAction[j] = srcs[j];
			}
			// if(i==0)
			// 	cout<<i<<" "<<mAction[2]<<endl;	
			mActions[i] = mAction;



		}
	}
}

Eigen::VectorXd
IntWindow::
getValueGradient(int index)
{
	p::object get_value_gradient;
	get_value_gradient = nn_sc_module[0].attr("get_value_gradient");

	Eigen::VectorXd state = mEnv->getState(index);
	Eigen::VectorXd valueGradient(state.size());

	p::tuple shape = p::make_tuple(state.size());
	np::dtype dtype = np::dtype::get_builtin<float>();
	np::ndarray state_np = np::empty(shape, dtype);
	float* dest = reinterpret_cast<float*>(state_np.get_data());
	for(int j=0;j<state.size();j++)
	{
		dest[j] = state[j];
	}

	p::object temp = get_value_gradient(state_np);
	np::ndarray valueGradient_np = np::from_object(temp);
	float* srcs = reinterpret_cast<float*>(valueGradient_np.get_data());
	for(int j=0;j<valueGradient.rows();j++)
	{
		valueGradient[j] = srcs[j];
	}
	return valueGradient;
}

void
IntWindow::
display()
{
	glClearColor(0.85, 0.85, 1.0, 1.0);
	// glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	initLights();
	mCamera->apply();

	std::vector<Character2D*> chars = mEnv->getCharacters();


	for(int i=0;i<chars.size();i++)
	{
		// if (i!=0)
		// 	continue;
		if(chars[i]->getTeamName() == "A")
			GUI::drawSkeleton(chars[i]->getSkeleton(), Eigen::Vector3d(1.0, 0.0, 0.0));
		// else
		// 	GUI::drawSkeleton(chars[i]->getSkeleton(), Eigen::Vector3d(0.0, 0.0, 1.0));

	}

	GUI::drawSkeleton(mEnv->floorSkel, Eigen::Vector3d(0.5, 1.0, 0.5));

	GUI::drawSkeleton(mEnv->ballSkel, Eigen::Vector3d(0.1, 0.1, 0.1));


	GUI::drawSkeleton(mEnv->wallSkel, Eigen::Vector3d(0.5,0.5,0.5));

	// Not simulated just for see
	GUI::drawSkeleton(redGoalpostSkel, Eigen::Vector3d(1.0, 1.0, 1.0));
	GUI::drawSkeleton(blueGoalpostSkel, Eigen::Vector3d(1.0, 1.0, 1.0));
	// cout<<"3333"<<endl;

	// std::string scoreString
	// = "Red : "+to_string((int)(mEnv->mAccScore[0] + mEnv->mAccScore[1]))+" |Blue : "+to_string((int)(mEnv->mAccScore[2]+mEnv->mAccScore[3]));

	std::string scoreString
	= "Red : "+to_string((int)(mEnv->mAccScore[0]));//+" |Blue : "+to_string((int)(mEnv->mAccScore[1]));
	// cout<<"444444"<<endl;


	GUI::drawStringOnScreen(0.2, 0.8, scoreString, true, Eigen::Vector3d::Zero());

	GUI::drawStringOnScreen(0.8, 0.8, to_string(mEnv->getElapsedTime()), true, Eigen::Vector3d::Zero());

	// cout<<"5555555"<<endl;


	GUI::drawSoccerLine(8, 6);

	glutSwapBuffers();
	if(mTakeScreenShot)
	{
		screenshot();
	}
	glutPostRedisplay();
}

std::string
IntWindow::
indexToStateString(int index)
{
	switch(index)
	{
		case 0:
			return "P_x";
		case 1:
			return "P_y";
		case 2:
			return "V_x";
		case 3:
			return "V_y";
		case 4:
			return "BP_x";
		case 5:
			return "BP_y";
		case 6:
			return "BV_x";
		case 7:
			return "BV_y";
		case 8:
			return "Kick";
		case 9:
			return "B_GP";
		case 10:
			return " ";
		case 11:
			return "B_GP";
		case 12:
			return " ";
		case 13:
			return "R_GP";
		case 14:
			return " ";
		case 15:
			return "R_GP";
		case 16:
			return " ";
		default:
			return "N";
	}
	return "N";
}

void
IntWindow::
drawValueGradient()
{
	int numStates = mEnv->mStates[0].size();
	// GUI::drawStringOnScreen(0.8, 0.8, to_string(mEnv->getElapsedTime()), true, Eigen::Vector3d::Zero());

	// double leftOffset = 0.02;
	// double rightOffset = 0.04;



	glPushMatrix();
	double boxSize = 1.0 / (numStates-1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,0.0,1.0,
			0.0,0.0, 0.0,
			0.0, 0.0 + 1.0,0.0);

	glTranslated(-0.5, -0.5, 0.0);

	GUI::drawValueGradientBox(mEnv->mStates[0], getValueGradient(0), boxSize);


	glPopMatrix();

	GLint w = glutGet(GLUT_WINDOW_WIDTH);
	GLint h = glutGet(GLUT_WINDOW_HEIGHT);

	for(int i=0;i<numStates;i++)
	{
		Eigen::Vector3d eyeToBox = Eigen::Vector3d(i * boxSize - 0.5, -0.5, 0.0);
		double fovx = mCamera->fovy * w / h;

		double boxAngleX = atan((eyeToBox[0] - boxSize/3.0)/(1.0 + boxSize)) / M_PI * 180.0;
		double boxAngleY = atan((eyeToBox[1]+boxSize)/(1.0 + boxSize)) / M_PI * 180.0; 

		// cout<<i<<endl;

		GUI::drawStringOnScreen_small(0.5 + boxAngleX/fovx, 0.5  + boxAngleY/mCamera->fovy, indexToStateString(i), Eigen::Vector3d::Zero());

	}


}


void
IntWindow::
mouse(int button, int state, int x, int y) 
{
	SimWindow::mouse(button, state, x, y);
}


void
IntWindow::
motion(int x, int y)
{
	SimWindow::motion(x, y);
}
