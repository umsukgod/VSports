#ifndef __VS_ENVIRONMENT_H__
#define __VS_ENVIRONMENT_H__
#include "Character2D.h"
#include "Character3D.h"
#include "BehaviorTree.h"
#include "../extern/ICA/plugin/MotionGenerator.h"
#include "../motion/BVHparser.h"
#include "../pyvs/Normalizer.h"

// #define _ID_P 0
// #define _ID_V 2
// #define _ID_BALL_P 4
// #define _ID_BALL_V 6
// #define _ID_KICKABLE 8
// #define _ID_GOALPOST_P 9
#define _ID_ALLY1_P 17
#define _ID_ALLY1_V 19
#define _ID_ALLY2_P 21
#define _ID_ALLY2_V 23
// #define _ID_OP_DEF_P 25
// #define _ID_OP_DEF_V 27
#define _ID_OP_ATK1_P 29
#define _ID_OP_ATK1_V 31
#define _ID_OP_ATK2_P 33
#define _ID_OP_ATK2_V 35
// #define _ID_FACING_V 37
// #define _ID_SLOWED 38

#define _ID_P 0
#define _ID_V 2
#define _ID_BALL_P 4
#define _ID_BALL_V 6
#define _ID_KICKABLE 8
#define _ID_GOALPOST_P 9
#define _ID_ALLY_P 17
#define _ID_ALLY_V 19
#define _ID_OP_DEF_P 21
#define _ID_OP_DEF_V 23
#define _ID_OP_ATK_P 25
#define _ID_OP_ATK_V 27
// #define _ID_FACING_V 29
#define _ID_SLOWED 29


// #define _ID_P 0
// #define _ID_V 2
// #define _ID_BALL_P 4
// #define _ID_BALL_V 6
// #define _ID_KICKABLE 8
// #define _ID_GOALPOST_P 9
#define _ID_OP_P 17
#define _ID_OP_V 19
// #define _ID_FACING_V 21
// #define _ID_SLOWED 22

enum BasketballState{
	POSITIONING, BALL_CATCH_1, DRIBBLE, BALL_CATCH_2
};

std::vector<int> getAvailableAction(BasketballState bs);

class BStateMachine
{
public:
	BStateMachine();
	BasketballState curState;
	int transition(int action, bool transitState = false); 	// return applied action. if it is not availble, it returns prev action
	int getStateWithInt();
	bool isAvailable(int action);
	std::vector<int> getAvailableActions();
	int prevAction;
};

class Environment;

class EnvironmentPackage
{
public:
	EnvironmentPackage();
	void saveEnvironment(Environment* env);
	void copyEnvironmentPackage(EnvironmentPackage* envPack);
	void restoreEnvironment(Environment* env);
	std::vector<Eigen::VectorXd> mActions;

	std::vector<bool> mPrevBallPossessions;
	std::vector<bool> mCurBallPossessions;


	// added for motion
	std::vector<Eigen::Vector3d> prevBallPositions;
	Eigen::Vector3d curBallPosition;
	Eigen::Vector3d curBallVelocity;

	Eigen::Vector3d criticalPoint_targetBallPosition;
	Eigen::Vector3d criticalPoint_targetBallVelocity;

	std::vector<int> prevContact;
	std::vector<int> curContact;

	int criticalPointFrame;
	int curFrame;

	Eigen::Vector3d mTargetBallPosition;


	std::vector<Eigen::VectorXd> mPrevActions;

	std::vector<int> mCurActionTypes;
	std::vector<Eigen::Vector3d> mPrevCOMs;

	Eigen::Vector3d mPrevBallPosition;

	std::vector<int> mCurCriticalActionTimes;

	// the player who carried the ball
	int mPrevPlayer;

	std::vector<bool> mDribbled;
	std::vector<bool> mLFootDetached;
	std::vector<bool> mRFootDetached;
	std::vector<Eigen::Vector3d> mObstacles;
	std::vector<double> mCurHeadingAngle;
	// std::vector<double**> mHeightMaps;

	std::vector<bool> mLFootContacting;
	std::vector<bool> mRFootContacting;

	std::vector<Eigen::Vector3d> mLLastFootPosition;
	std::vector<Eigen::Vector3d> mRLastFootPosition;
};


typedef std::pair<std::string, Eigen::Vector3d> GoalpostInfo;
class AgentEnvWindow;
class Environment
{
public:
	Environment(int control_Hz, int simulation_Hz, int numChars, std::string bvh_path, std::string nn_path);
	void resetCharacterPositions();
	void initGoalposts();
	void initFloor();
	void initBall();

	void step();

	void stepAtOnce();

	void reset();
	bool isTerminalState();


	// For DeepRL
	Eigen::VectorXd getState(int index);
	Eigen::VectorXd getLocalState(int index);


	Eigen::VectorXd getNormalizedState(int index);

	double getReward(int index, bool verbose = true);
	std::vector<double> getRewards();

	Eigen::VectorXd getAction(int index){return mActions[index];}
	std::vector<Eigen::VectorXd> getActions(){return mActions;}

	void setAction(int index, const Eigen::VectorXd& a);
	void applyAction(int index);

	int getNumState(int index = 0){return getState(index).size();}
	// int getNumAction(int index = 0){return getAction(index).rows();}
	int getNumAction(int index = 0){return getAction(index).rows();}
// 
	const dart::simulation::WorldPtr& getWorld(){return mWorld;}

	Character3D* getCharacter(int index){return mCharacters[index];}
	std::vector<Character3D*> getCharacters(){return mCharacters;}

	int getControlHz(){return mControlHz;}
	int getSimulationHz(){return mSimulationHz;}

	double getElapsedTime(){return mTimeElapsed;}

	std::vector<int> getCollidingWall(dart::dynamics::SkeletonPtr skel, double radius);

	void handleWallContact(dart::dynamics::SkeletonPtr skel, double radius, double me = 1.0);
	void handleBallContact(int index, double radius, double me = 0.5);
	void handlePlayerContact(int index1, int index2, double me = 0.5);
	void handlePlayerContacts(double me = 0.5);

	void boundBallVelocitiy(double maxVel);
	void dampBallVelocitiy(double dampPower);

	// Eigen::VectorXd normalizeNNState(Eigen::VectorXd state);
	// Eigen::VectorXd unNormalizeNNState(Eigen::VectorXd normalizedState);

	void setVState(int index, Eigen::VectorXd latentState);

	void initBehaviorTree();
	Eigen::VectorXd getActionFromBTree(int index);

	void setHardcodedAction(int index);

	std::vector<int> getAgentViewImg(int index);

	int getNumBallTouch(){return mNumBallTouch;}

	void updatePrevBallPositions(Eigen::Vector3d newBallPosition);
	void updatePrevContacts(int index, Eigen::Vector2d handContacts);
	bool isCriticalPoint();

	bool isCriticalAction(int actionType);

public:
	dart::simulation::WorldPtr mWorld;
	int mNumChars;

	double mTimeElapsed;
	int mControlHz;
	int mSimulationHz;

	std::vector<Character3D*> mCharacters;

	dart::dynamics::SkeletonPtr floorSkel;
	dart::dynamics::SkeletonPtr ballSkel;
	dart::dynamics::SkeletonPtr wallSkel;

	std::vector<GoalpostInfo> mGoalposts;
	std::vector<Eigen::VectorXd> mActions;
	std::vector<Eigen::VectorXd> mSimpleStates;
	std::vector<Eigen::VectorXd> mForces;

	double floorDepth = 0.0;

	bool mIsTerminalState;

	Eigen::VectorXd mScoreBoard;

	Eigen::VectorXd mAccScore;

	Eigen::VectorXd mTouch;

	int mNumIterations;

	std::vector<Eigen::VectorXd> mStates;
	std::vector<Eigen::VectorXd> mLocalStates;
	std::vector<Eigen::VectorXd> mVStates;

	std::vector<BNode*> mBTs;
	double maxVel = 0.8;

	std::vector<double> mFacingVels;

	std::vector<int> mKicked;
	double mSlowDuration;

	void reconEnvFromState(int index, Eigen::VectorXd curLocalState);


	int mNumBallTouch;

	int endTime;

	std::vector<bool> mPrevBallPossessions;
	std::vector<bool> mCurBallPossessions;

	Normalizer* mNormalizer;

	std::map<int, std::string> actionNameMap;






	// added for motion
	std::vector<Eigen::Vector3d> prevBallPositions;
	Eigen::Vector3d curBallPosition;
	Eigen::Vector3d curBallVelocity;

	Eigen::Vector3d criticalPoint_targetBallPosition;
	Eigen::Vector3d criticalPoint_targetBallVelocity;
	Eigen::Vector3d computeBallPosition();
	void initDartNameIdMapping();

	void initMotionGenerator(std::string dataPath);

	std::map<std::string, int> dartNameIdMap;
	std::vector<int> prevContact;
	std::vector<int> curContact;


    ICA::dart::MotionGenerator* mMotionGenerator;

    BVHparser* mBvhParser;
	void initCharacters(std::string bvhPath);

	int criticalPointFrame;
	int curFrame;

	Eigen::Vector3d mTargetBallPosition;

	void resetTargetBallPosition();
	Eigen::Vector3d getTargetBallGlobalPosition();

	std::vector<Eigen::VectorXd> mPrevActions;
	std::vector<int> mPrevActionTypes;

	std::vector<int> mCurActionTypes;
	std::vector<Eigen::Vector3d> mPrevCOMs;

	Eigen::Vector3d mPrevBallPosition;

	std::vector<int> mCurCriticalActionTimes;
	void computeCriticalActionTimes();

	// the player who carried the ball
	int mPrevPlayer;

	std::vector<bool> mDribbled;

	bool mIsFoulState;

	bool isFoulState();

	void goToPrevSituation();

	// std::vector<bool> mLFootDetached;
	// std::vector<bool> mRFootDetached;

	std::vector<BStateMachine*> bsm;

	void genObstacleNearCharacter();
	void removeOldestObstacle();

	void genObstaclesToTargetDir(int numObstacles);


	std::vector<Eigen::Vector3d> mObstacles;

	std::vector<double> mCurHeadingAngle;

	std::vector<double**> mHeightMaps;
	int mNumGrids = 32;
	double mMapRange = 8.0; 
	std::vector<bool> mLFootDetached;
	std::vector<bool> mRFootDetached;

	void updateHeightMap(int index);
	std::vector<Eigen::Vector3d> getHeightMapGrids(int index);


	std::vector<float> getHeightMapState(int index);

	std::vector<EnvironmentPackage*> mPrevEnvSituations;

	void saveEnvironment();
	void goBackEnvironment();

	Eigen::Vector3d computeHandBallPosition(int index);

	std::vector<bool> mLFootContacting;
	std::vector<bool> mRFootContacting;

	std::vector<Eigen::Vector3d> mLLastFootPosition;
	std::vector<Eigen::Vector3d> mRLastFootPosition;


	bool ballGravityChecker(int index);
	std::vector<double> prevFreeBallPositions;

	std::vector<double> mActionGlobalBallPosition;
	std::vector<Eigen::Vector3d> mActionGlobalBallVelocity;

	std::vector<bool> mChangeContactIsActive;


	// std::vector<Eigen::Isometry3d> mPrevLHandTranform;
	// std::vector<Eigen::Isometry3d> mPrevRHandTranform;
	// int mCurPlayer;

	Eigen::VectorXd dribbleDefaultVec;

	Eigen::Isometry3d getRootT(int index);
	// AgentEnvWindow* mWindow;
};
double getFacingAngleFromLocalState(Eigen::VectorXd curState);
Eigen::VectorXd localStateToOriginState(Eigen::VectorXd localState, int mNumChars=6);
double getBounceTime(double startPosition, double startVelocity, double upperbound);
#endif