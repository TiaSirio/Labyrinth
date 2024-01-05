// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"


namespace utils {
	struct DescriptorPoolSize {
		int numberOfUniformBlocksInPool;
		int numberOfTexturesInPool;
		int numberOfSetsInPool;

		DescriptorPoolSize() : numberOfUniformBlocksInPool(0), numberOfTexturesInPool(0), numberOfSetsInPool(0) {};

		//The below functions manage the Descriptor pool sizes (only one add for all because the descriptor set used for object is only one)
		void addObject() {
			numberOfTexturesInPool++;
			numberOfUniformBlocksInPool++;
			numberOfSetsInPool++;
		}

		void addMultipleInstanceObject(int numberOfInstances) {
			for (int i = 0; i < numberOfInstances; i++) {
				numberOfTexturesInPool++;
				numberOfUniformBlocksInPool++;
				numberOfSetsInPool++;
			}
		}

		void addDescriptorSet() {
			numberOfUniformBlocksInPool++;
			numberOfSetsInPool++;
		}
	};

	DescriptorPoolSize descriptorPoolSize;

	void addObject() {
		descriptorPoolSize.addObject();
	}

	void addMultipleInstanceObject(int numberOfInstances) {
		descriptorPoolSize.addMultipleInstanceObject(numberOfInstances);
	}

	void addDescriptor() {
		descriptorPoolSize.addDescriptorSet();
	}
};

//TorchPos is to define where the light comes from
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 torchPos;
	alignas(16) glm::vec3 eyePos;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 normal;
	alignas(16) float roughness;
};


struct DescriptorSetLayoutObject {
	DescriptorSetLayout descriptorSetLayout;
	std::vector<uint32_t> set;
	std::vector<VkDescriptorType> vkDescriptorType;
	std::vector<VkShaderStageFlagBits> vkShaderStageFlagBits;
	std::vector<DescriptorSetElementType> descriptorSetElementType;
	uint64_t sizeOfUniformBufferObject;
	uint64_t sizeOfGlobalUniformBufferObject;

	//Functions to manage the various values needed from a DescriptorSetLayout
	DescriptorSetLayoutObject(std::vector<uint32_t> set, std::vector<VkDescriptorType> vkDescriptorType, std::vector<VkShaderStageFlagBits> vkShaderStageFlagBits) {
		this->set = set;
		this->vkDescriptorType = vkDescriptorType;
		this->vkShaderStageFlagBits = vkShaderStageFlagBits;
		getCorrectDescriptorSetElementType();
		sizeOfUniformBufferObject = sizeof(UniformBufferObject);
		sizeOfGlobalUniformBufferObject = sizeof(GlobalUniformBufferObject);
	}

	void getCorrectDescriptorSetElementType() {
		this->descriptorSetElementType.resize(set.size());
		for (int i = 0; i < vkDescriptorType.size(); i++) {
			if (vkDescriptorType[i] == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				this->descriptorSetElementType[i] = UNIFORM;
			}
			else if (vkDescriptorType[i] == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				this->descriptorSetElementType[i] = TEXTURE;
			}
			else {
				this->descriptorSetElementType[i] = UNIFORM;
			}
		}
	}

	void cleanup() {
		descriptorSetLayout.cleanup();
	}
};


//Manage the descriptor pool size and cleanup function
struct Object {
	Model model;
	Texture texture;
	DescriptorSet descriptorSet;

	Object() {
		utils::addObject();
	}

	void cleanup() {
		descriptorSet.cleanup();
		texture.cleanup();
		model.cleanup();
	}
};

//Manage multiple objects
struct MultipleObject {
	Model model;
	Texture texture;
	std::vector<DescriptorSet> descriptorSets;
	int numberOfinstances;

	MultipleObject(int numberOfInstances) {
		numberOfinstances = numberOfInstances;
		utils::addMultipleInstanceObject(numberOfInstances);
	}

	void cleanup() {
		for (DescriptorSet descriptorSet : descriptorSets) {
			descriptorSet.cleanup();
		}
		texture.cleanup();
		model.cleanup();
	}
};

//Player
	//Position
static glm::vec3 pos = glm::vec3(500.0f, 199.5f, -495.0f);
static glm::vec3 oldPos = glm::vec3(500.0f, 199.5f, -495.0f);
static glm::vec3 savePos = glm::vec3(0);
//Visual
static float lookYaw = 0.0f;
static float lookYawSaved = 0.0f;
static float lookPitch = 0.0f;
static float lookPitchSaved = 0.0f;
static float lookRoll = 0.0f;
//Win
static bool win = false;
static bool firstWin = false;
//Jump
static bool jump = false;
static bool jumpDown = false;
//Camera
static glm::vec3 CamPos = glm::vec3(0);
//static glm::mat3 CamDir = glm::mat3(0);
static glm::mat4 CharacterPos = glm::mat4(0);
//static glm::vec3 lineOfSightDirection = glm::vec3(0);

//Environment
	//User can pass
static std::vector<int> doorOpenOrNot = { 0,0,0,0,0 };
//Used for controls on door closing
static std::vector<int> doorAlreadyOpened = { 0,0,0,0,0 };
//User has the possibility to do something to open the door
static std::vector<int> userCouldPassThroughDoors = { 0,0,0,0,0 };
//Gold and Copper key - User could open the door with the keys
static std::vector<int> doorCouldBeOpened = { 0,0 };
//User could open the door with the levers
static std::vector<int> leverCouldBeUsed = { 0,0,0 };
//User has used a lever
static std::vector<int> leverUsedOrNot = { 0,0,0 };
//For stats
static std::vector<int> leverUsedAtLeastOnce = { 0,0,0 };
//User could pick a key
static std::vector<int> keyCouldBeTaken = { 0,0 };
//User has pick a key
static std::vector<int> keyTakenOrNot = { 0,0 };
//User has pick the powerUp
static float powerUpTakenOrNot = 0.0f;
//Value to rotate the powerUp
static int rotatingPowerUp = 0;
//Float value to be incremented slowly to slowly open the door
static std::vector<float> slowlyOpenDoors = { 0.0f,0.0f,0.0f,0.0f,0.0f };
//Position of the torch to see through the labyrinth
static glm::vec3 torchPos = glm::vec3(0.0f, 0.65f, -4.42f);
static glm::vec3 torchPosStatic = glm::vec3(0.0f, 0.65f, -4.42f);
//Position of the torch object
static glm::vec3 objectTorchPos = glm::vec3(0.0f, 0.65f, -4.42f);
//User could take the torch
static bool torchCouldBeTaken = false;
//User has taken the torch
static bool torchTaken = false;
//Position of the gold key
static glm::vec3 goldKeyPos = glm::vec3(0.0f, 0.0f, 0.0f);
static glm::vec3 goldKeyPosStatic = glm::vec3(10.0f, 0.0f, -8.0f);
//Position of the copper key
static glm::vec3 copperKeyPos = glm::vec3(0.0f, 0.0f, 0.0f);
static glm::vec3 copperKeyPosStatic = glm::vec3(15.0f, 0.0f, 3.0f);
//Save the position of the keys
static glm::vec3 savePosKey = glm::vec3(0.0f, 0.0f, 0.0f);

//Static postion of the key for stats
static glm::vec3 keyPositionStats = glm::vec3(500.0f, 200.5f, 500.0f);


//Game input
	//User see the tutorial
static bool showTutorial = true;

//Whenever there is a state + first, is done as a semaphore like logic to not spam key input of the user
static bool stateF11 = false;
static bool firstF11 = true;

static bool stateX = false;
static bool firstX = true;

static bool stateL1 = false;
static bool firstL1 = true;

//Controller
	//Variable that check if the controller is plugged or not
int controllerPlugged = 0;
//Check the movement, using the controller
int axesCount;
const float* axes;
//Check the interaction with the buttons of the controller
int buttonCount;
const unsigned char* buttons;
//Set of variables to check the differences between if a controller is plugged or a keyboard
static bool controllerInput = false;
static bool keyboardInput = true;
static bool switchInput = false;

//Tutorial
	//Variable that let the user see the correct tutorial text
static std::vector<int> tutorialElements = { 1,1,1,1,1,1,1,1 };
//Variable that let the user see the correct next button of the tutorial
static std::vector<int> tutorialNextElements = { 1,1 };
//Manage the skip.
static int skipElement = 1;
//To move slowly the tutorial texts
static std::vector<float> moveSlowlyTutorial = { 0.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f };

//Variable that let the user see the correct tutorial text
static std::vector<int> tutorialElementsController = { 1,1,1,1,1,1,1,1 };
//Variable that let the user see the correct next button of the tutorial
static std::vector<int> tutorialNextElementsController = { 1,1 };
//Manage the skip.
static int skipElementController = 1;
//To move slowly the tutorial texts for controller
static std::vector<float> moveSlowlyTutorialController = { 0.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f };

//Variable that let the user see the next element of the tutorial
static bool seeTheNextElementOfTutorial = true;

//Saving the element of the tutorial if the user want to see it again
static std::vector<int> savedTutorialElements = { 1,1,1,1,1,1,1,1,1 };
static std::vector<int> savedTutorialNextElements = { 1,1 };
//To let the user not spam keyboard input
static bool stateTutorial = false;
static bool firstTutorial = true;
static bool stateTutorialAgain = false;
static bool firstTutorialAgain = true;
static bool doneTutorialAgain = false;
static bool restartTutorialWithinIt = false;
static bool stateInteraction = false;
static bool firstInteraction = false;
//To check first time the user do the tutorial and place it in its initial position
static bool firstTimeDoingTheTutorial = true;

//Window dimensions
int width = 0;
int height = 0;

//Mouse position
double xpos = 0;
double ypos = 0;

//Menu values
static bool menuOpen = true;
static bool enterTheGame = false;
static bool enterTheTutorial = false;

//Stats
	//Variable that manage the stats texts
static int enterInTheStats = 1;
//Stats for the doors
static std::vector<int> statsForDoorsAtTheEnd = { 1,1,1 };
//Stats for keys
static int statsForKeysAtTheEnd = 1;
//Stats for fruit
static std::vector<int> statsForFruitAtTheEnd = { 1,1 };
//Stats for levers
static std::vector<int> statsForLeversAtTheEnd = { 1,1,1 };

stbi_uc* map;
int mapWidth, mapHeight;

const float checkRadius = 0.1;
const int checkSteps = 12;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";

/// <summary>
/// Using the position the User see if he can pass through a door or is blocked
/// </summary>
/// <param name="pixDoorX">Integer position of the x-axis player</param>
/// <param name="pixDoorY">Integer position of the x-axis player</param>
/// <returns>1, if the user could pass through the door, otherwise returns 0</returns>
int checkCorrectDoor(int pixDoorX, int pixDoorY) {
	if (pixDoorX == 4 && pixDoorY == 3) {
		//Lever door - third lever
		//return doorOpenOrNot[0];
		return userCouldPassThroughDoors[0];
	}
	else if (pixDoorX == 7 && pixDoorY == 8) {
		//Copper key (last door)
		//return doorOpenOrNot[1];
		return userCouldPassThroughDoors[1];
	}
	else if (pixDoorX == 9 && pixDoorY == 3) {
		//Lever door - second lever
		//return doorOpenOrNot[2];
		return userCouldPassThroughDoors[2];
	}
	else if (pixDoorX == 4 && pixDoorY == -2) {
		//Lever door - first lever (first door)
		//return doorOpenOrNot[3];
		return userCouldPassThroughDoors[3];
	}
	else {
		//Gold key door
		//return doorOpenOrNot[4];
		return userCouldPassThroughDoors[4];
	}
	//x = 12; y = 4
}

/// <summary>
/// Check the movement of the user on the map.
/// </summary>
/// <param name="x">X position of the user.</param>
/// <param name="y">Y position of the user.</param>
/// <returns>If the user can access the current x,y value or not.</returns>
bool canStepPoint(float x, float y) {
	//To have a precision of the square in which the player is
	int roundX = round(x);
	int roundY = round(y);
	//To have a higher precision
	float roundFirstDecimalX = round(x * 10.0) / 10.0;
	float roundFirstDecimalY = round(y * 10.0) / 10.0;


	//It calculate the pixel in which the user is, according to a height map present in the resources.
	int pixX = round(fmax(0.0f, fmin(mapWidth - 1, (x + 16.9) * mapWidth / 33.8)));
	int pixY = round(fmax(0.0f, fmin(mapHeight - 1, (y + 16.9) * mapHeight / 33.8)));
	//Return the value of the zone in which the user is and the logic of the interaction of the game is based on that value.
	int pix = (int)map[mapWidth * pixY + pixX];
	//std::cout << pixX << " " << pixY << '\n';
	//std::cout << pixX << " " << pixY << " " << x << " " << y << " \t P = " << pix << "\n";

	if (roundX == 10 && roundY == -8 && keyTakenOrNot[0] == 0) {
		//GoldKey
		keyCouldBeTaken[0] = 1;
	}
	else {
		keyCouldBeTaken[0] = 0;
	}
	//For all the interaction except for the keys, there is a double control, because the user could interact with something only at a certain distance
	if (roundX == 12 && (roundFirstDecimalY >= 3.4 && roundFirstDecimalY <= 4) && keyTakenOrNot[0] == 1 && doorOpenOrNot[4] == 0) {
		//GoldKey
		doorCouldBeOpened[0] = 1;
	}
	else {
		doorCouldBeOpened[0] = 0;
	}
	if (roundX == 15 && roundY == 3 && keyTakenOrNot[1] == 0) {
		//CopperKey
		keyCouldBeTaken[1] = 1;
	}
	else {
		keyCouldBeTaken[1] = 0;
	}
	if ((roundFirstDecimalX >= 6.4 && roundFirstDecimalX <= 7) && roundY == 8 && keyTakenOrNot[1] == 1 && doorOpenOrNot[1] == 0) {
		//CopperKey
		doorCouldBeOpened[1] = 1;
	}
	else {
		doorCouldBeOpened[1] = 0;
	}
	//x = 3.3, y = -1.5
	if ((roundFirstDecimalX >= 3 && roundFirstDecimalX <= 3.6) && (roundFirstDecimalY >= -1.5 && roundFirstDecimalY <= -1.1) /* && leverUsedOrNot[2] == 0 && doorOpenOrNot[3] == 0*/) {
		if (torchTaken) {
			//First lever
			leverCouldBeUsed[2] = 1;
		}
	}
	else {
		leverCouldBeUsed[2] = 0;
	}
	//x = 8.3, y = 3.5
	if ((roundFirstDecimalX >= 8.0 && roundFirstDecimalX <= 8.6) && (roundFirstDecimalY >= 3.5 && roundFirstDecimalY <= 4.1)/* && leverUsedOrNot[1] == 0 && doorOpenOrNot[2] == 0*/) {
		//Second lever
		leverCouldBeUsed[1] = 1;
	}
	else {
		leverCouldBeUsed[1] = 0;
	}
	//x = 2.5, y = 2.2
	if ((roundFirstDecimalX >= 1.9 && roundFirstDecimalX <= 2.5) && (roundFirstDecimalY >= 1.9 && roundFirstDecimalY <= 2.5)/* && leverUsedOrNot[0] == 0 && doorOpenOrNot[0] == 0*/) {
		//Third lever
		leverCouldBeUsed[0] = 1;
	}
	else {
		leverCouldBeUsed[0] = 0;
	}
	//x = -4.0, y = 12.0
	if ((roundFirstDecimalX >= -4.2 && roundFirstDecimalX <= -3.8) && (roundFirstDecimalY >= 11.8 && roundFirstDecimalY <= 12.2) && powerUpTakenOrNot == 0.0f) {
		powerUpTakenOrNot = 0.8f;
	}
	if (roundX == 0 && (roundFirstDecimalY >= -4.5 && roundFirstDecimalY <= -4.1)) {
		torchCouldBeTaken = true;
	}
	else {
		torchCouldBeTaken = false;
	}
	//When the value of pix is > 200, it means we have reached a door (the highest object in the map) and we must check if thedoor is open or not
	if (pix > 200) {
		//Function to check the closest door and check if open or not
		return checkCorrectDoor(roundX, roundY) == 1;
	}
	//If the last door is open
	if (doorOpenOrNot[1] == 1) {
		//Check final area and set win to true if arrived
		if ((roundX == 7 && roundY == 8) || (roundX == 8 && roundY == 8)) {
			if ((roundFirstDecimalX >= 8 && roundFirstDecimalX <= 9) && roundY == 8) {
				win = true;
				firstWin = true;
			}
			//Only in the final are we return pix >= 0, because the height map is the same color as the outer area of the labyrinth
			return pix >= 0;
		}
	}
	//pix > 90 is a standard value that return the paths of the labyrinth
	return pix > 90;
}

//We keep check if the user could step or not, according to a height map
bool canStep(float x, float y) {
	for (int i = 0; i < checkSteps; i++) {
		if (!canStepPoint(x + cos(6.2832 * i / (float)checkSteps) * checkRadius,
			y + sin(6.2832 * i / (float)checkSteps) * checkRadius)) {
			return false;
		}
	}
	return true;
}

// MAIN ! 
class MyProject : public BaseProject {
protected:
	// Here you list all the Vulkan objects you need:

	//Value needed for the descriptorSetLayout
	std::vector<uint32_t> set0 = { 0 };
	std::vector<VkDescriptorType> vkDescriptorType0 = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
	std::vector<VkShaderStageFlagBits> vkShaderStageFlagBits0 = { VK_SHADER_STAGE_ALL_GRAPHICS };

	DescriptorSetLayoutObject descriptorSetLayoutGlobal = DescriptorSetLayoutObject(set0, vkDescriptorType0, vkShaderStageFlagBits0);

	//Value needed for the descriptorSetLayout
	std::vector<uint32_t> set1 = { 0,1 };
	std::vector<VkDescriptorType> vkDescriptorType1 = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	//The first value is to all because it is used for the roughness of the material
	std::vector<VkShaderStageFlagBits> vkShaderStageFlagBits1 = { VK_SHADER_STAGE_ALL_GRAPHICS, VK_SHADER_STAGE_FRAGMENT_BIT };

	DescriptorSetLayoutObject descriptorSetLayoutObject = DescriptorSetLayoutObject(set1, vkDescriptorType1, vkShaderStageFlagBits1);

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Object mainCharacter;

	Object floorObject;
	Object ceilingObject;

	Object wallEastObject;
	Object wallNorthObject;
	Object wallWestObject;
	Object wallSouthObject;

	Object goldKeyObject;
	Object copperKeyObject;

	Object goldKeyHoleObject;
	Object copperKeyHoleObject;

	Object doorBorders;
	MultipleObject doors = MultipleObject(5);

	MultipleObject levers = MultipleObject(3);

	Object powerUp;
	Object powerUpBase;

	Object winText;
	Object winPlayAgain;
	Object winPlayAgainController;

	Object torch;

	Object tutorial;
	Object interaction;
	Object movement;
	Object visual;
	Object windowTutorial;
	Object restart;
	Object tutorialAgain;
	Object next;
	Object end;
	Object skip;
	Object jumpObject;

	//Tutorial Controller
	Object interactionController;
	Object movementController;
	Object visualController;
	Object restartController;
	Object tutorialAgainController;
	Object nextController;
	Object endController;
	Object skipController;
	Object jumpController;

	//Menu
	Object welcomeTextInTheGame;
	Object startPlayTheGame;
	Object goToSeeTheTutorial;

	//Stats
	Object statsText;
	Object statsObjects;
	Object statsEnter;
	Object statsEnterController;
	Object statsExit;
	Object statsExitController;

	//Stats value
	Object statsValueDoors3;
	Object statsValueDoors4;
	Object statsValueDoors5;
	Object statsValueKeys;
	Object statsValueFruit0;
	Object statsValueFruit1;
	Object statsValueLevers1;
	Object statsValueLevers2;
	Object statsValueLevers3;

	DescriptorSet DS_global;


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, title and initial background
		windowWidth = 800;//1920;
		windowHeight = 600;//1080;
		windowTitle = "Labyrinth Escape";
		initialBackgroundColor = { 1.0f, 0.3f, 0.0f, 1.0f };

		//Done for DS_global
		utils::addDescriptor();

		// Descriptor pool sizes
		uniformBlocksInPool = utils::descriptorPoolSize.numberOfUniformBlocksInPool;
		texturesInPool = utils::descriptorPoolSize.numberOfTexturesInPool;
		setsInPool = utils::descriptorPoolSize.numberOfSetsInPool;
	}

	//
	void showTutorialInfo() {
		std::cout << "\nTutorial:";
		std::cout << "\nGoal of the game: reach the end of the labyrinth, behind the bronze room";
		std::cout << "\nMovement keys:" << "\n\t Move left: A" << "\n\t Move right: D" << "\n\t Move forward: W" << "\n\t Move backward: S";
		std::cout << "\nVisual keys:" << "\n\t Look left: Left" << "\n\t Look right: Right" << "\n\t Look forward: Top" << "\n\t Look backward: Bottom";
		std::cout << "\nWindow keys:" << "\n\t FullScreen on and off: F11";
		std::cout << "\nOther keys:" << "\n\t Interact with object: E - Square" << "\n\t Restart game when ended: P - Share" << "\n\t See again the tutorial: T - Option" << "\n\t See the stats: X - R1\n";
	}

	// Here you load and setup all your Vulkan objects
	void localInit() {

		//Init of the two descriptor set layout (0,1)
		descriptorSetLayoutInit(&descriptorSetLayoutGlobal);
		descriptorSetLayoutInit(&descriptorSetLayoutObject);

		localPipelineInit();

		//Character
		objectInit(&mainCharacter, MODEL_PATH + "Character/Character.obj", TEXTURE_PATH + "SkinColor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Floor and ceiling
		objectInit(&floorObject, MODEL_PATH + "FloorAndCeiling/Floor.obj", TEXTURE_PATH + "Floor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&ceilingObject, MODEL_PATH + "FloorAndCeiling/Ceiling.obj", TEXTURE_PATH + "Ceiling.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Walls
		objectInit(&wallEastObject, MODEL_PATH + "Wall/WallEast.obj", TEXTURE_PATH + "Walls.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&wallNorthObject, MODEL_PATH + "Wall/WallNorth.obj", TEXTURE_PATH + "Walls.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&wallSouthObject, MODEL_PATH + "Wall/WallSouth.obj", TEXTURE_PATH + "Walls.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&wallWestObject, MODEL_PATH + "Wall//WallWest.obj", TEXTURE_PATH + "Walls.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Gold key
		objectInit(&goldKeyObject, MODEL_PATH + "Key/GoldKey.obj", TEXTURE_PATH + "GoldKeyColor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&goldKeyHoleObject, MODEL_PATH + "KeyHole/GoldKeyHole.obj", TEXTURE_PATH + "GoldKeyColor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Copper key
		objectInit(&copperKeyObject, MODEL_PATH + "Key/CopperKey.obj", TEXTURE_PATH + "CopperKeyColor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&copperKeyHoleObject, MODEL_PATH + "KeyHole/CopperKeyHole.obj", TEXTURE_PATH + "CopperKeyColor.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Doors
		objectInit(&doorBorders, MODEL_PATH + "Door/DoorsBorder.obj", TEXTURE_PATH + "DoorBorder.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		multipleInstanceObjectInit(&doors, MODEL_PATH + "Door/Door.obj", TEXTURE_PATH + "Door.png", descriptorSetLayoutObject.descriptorSetLayout, 5, descriptorSetLayoutObject);

		//Levers
		multipleInstanceObjectInit(&levers, MODEL_PATH + "Lever/Lever.obj", TEXTURE_PATH + "Levers.png", descriptorSetLayoutObject.descriptorSetLayout, 3, descriptorSetLayoutObject);

		//Power up
		objectInit(&powerUp, MODEL_PATH + "PowerUp/Juice.obj", TEXTURE_PATH + "Juice.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&powerUpBase, MODEL_PATH + "PowerUp/PowerUpBase.obj", TEXTURE_PATH + "Ceiling.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Win
		objectInit(&winText, MODEL_PATH + "Win/WinText.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&winPlayAgain, MODEL_PATH + "Win/WinPlayAgain.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&winPlayAgainController, MODEL_PATH + "Win/WinPlayAgainController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Torch
		objectInit(&torch, MODEL_PATH + "Torch/Torch2.obj", TEXTURE_PATH + "Torch.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Tutorial
		objectInit(&tutorial, MODEL_PATH + "Tutorial/TutorialFirstText.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&interaction, MODEL_PATH + "Tutorial/Interaction.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&movement, MODEL_PATH + "Tutorial/Movement.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&visual, MODEL_PATH + "Tutorial/Visual.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&windowTutorial, MODEL_PATH + "Tutorial/Window.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&restart, MODEL_PATH + "Tutorial/Restart.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&tutorialAgain, MODEL_PATH + "Tutorial/TutorialAgain.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&next, MODEL_PATH + "Tutorial/Next.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&end, MODEL_PATH + "Tutorial/End.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&skip, MODEL_PATH + "Tutorial/Skip.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&jumpObject, MODEL_PATH + "Tutorial/Jump.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Tutorial controller
		objectInit(&interactionController, MODEL_PATH + "Tutorial/InteractionController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&movementController, MODEL_PATH + "Tutorial/MovementController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&visualController, MODEL_PATH + "Tutorial/VisualController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&restartController, MODEL_PATH + "Tutorial/RestartController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&tutorialAgainController, MODEL_PATH + "Tutorial/TutorialAgainController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&nextController, MODEL_PATH + "Tutorial/NextController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&endController, MODEL_PATH + "Tutorial/EndController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&skipController, MODEL_PATH + "Tutorial/SkipController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&jumpController, MODEL_PATH + "Tutorial/JumpController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Menu
		objectInit(&welcomeTextInTheGame, MODEL_PATH + "Menu/Tutorial.obj", TEXTURE_PATH + "MenuText.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&startPlayTheGame, MODEL_PATH + "Menu/Play.obj", TEXTURE_PATH + "MenuText.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&goToSeeTheTutorial, MODEL_PATH + "Menu/StartTutorial.obj", TEXTURE_PATH + "MenuText.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Stats
		objectInit(&statsText, MODEL_PATH + "Stats/Stats.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsObjects, MODEL_PATH + "Stats/StatsKey.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsEnter, MODEL_PATH + "Stats/StatsEnter.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsEnterController, MODEL_PATH + "Stats/StatsEnterController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsExit, MODEL_PATH + "Stats/StatsExit.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsExitController, MODEL_PATH + "Stats/StatsExitController.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);

		//Stats value
		objectInit(&statsValueDoors3, MODEL_PATH + "Stats/StatsValueDoors3.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueDoors4, MODEL_PATH + "Stats/StatsValueDoors4.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueDoors5, MODEL_PATH + "Stats/StatsValueDoors5.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueKeys, MODEL_PATH + "Stats/StatsValueKeys.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueFruit0, MODEL_PATH + "Stats/StatsValueFruit0.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueFruit1, MODEL_PATH + "Stats/StatsValueFruit1.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueLevers1, MODEL_PATH + "Stats/StatsValueLevers1.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueLevers2, MODEL_PATH + "Stats/StatsValueLevers2.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);
		objectInit(&statsValueLevers3, MODEL_PATH + "Stats/StatsValueLevers3.obj", TEXTURE_PATH + "Tutorial.png", descriptorSetLayoutObject.descriptorSetLayout, descriptorSetLayoutObject);


		//Init of the global descriptor set used
		descriptorSetInit(&DS_global, descriptorSetLayoutGlobal.descriptorSetLayout, descriptorSetLayoutGlobal);

		//Load height map
		map = stbi_load((TEXTURE_PATH + "displ.png").c_str(),
			&mapWidth, &mapHeight,
			NULL, 1);
		if (!map) {
			std::cout << (TEXTURE_PATH + "displ.png").c_str() << "\n";
			throw std::runtime_error("failed to load map image!");
		}
		std::cout << "map -> size: " << mapWidth
			<< "x" << mapHeight << "\n";

		//Show in the cmd a tutorial of the commands
		showTutorialInfo();
	}

	void localPipelineInit() {
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", { &descriptorSetLayoutGlobal.descriptorSetLayout, &descriptorSetLayoutObject.descriptorSetLayout });
	}





	void localPipelineCleanup() {
		//Here clean all the pipelines.
		P1.cleanup();
	}

	void descriptorLayoutsCleanup() {
		descriptorSetLayoutGlobal.cleanup();
		descriptorSetLayoutObject.cleanup();
	}

	void descriptorsCleanup() {
		DS_global.cleanup();
	}

	void objectsCleanup() {
		mainCharacter.cleanup();

		floorObject.cleanup();
		ceilingObject.cleanup();

		powerUp.cleanup();
		powerUpBase.cleanup();

		wallEastObject.cleanup();
		wallNorthObject.cleanup();
		wallSouthObject.cleanup();
		wallWestObject.cleanup();

		goldKeyObject.cleanup();
		goldKeyHoleObject.cleanup();

		copperKeyObject.cleanup();
		copperKeyHoleObject.cleanup();

		doorBorders.cleanup();
		doors.cleanup();

		levers.cleanup();

		winText.cleanup();
		winPlayAgain.cleanup();
		winPlayAgainController.cleanup();

		torch.cleanup();

		tutorial.cleanup();
		interaction.cleanup();
		movement.cleanup();
		visual.cleanup();
		windowTutorial.cleanup();
		restart.cleanup();
		tutorialAgain.cleanup();
		next.cleanup();
		end.cleanup();
		skip.cleanup();
		jumpObject.cleanup();

		interactionController.cleanup();
		movementController.cleanup();
		visualController.cleanup();
		restartController.cleanup();
		tutorialAgainController.cleanup();
		nextController.cleanup();
		endController.cleanup();
		skipController.cleanup();
		jumpController.cleanup();

		welcomeTextInTheGame.cleanup();
		startPlayTheGame.cleanup();
		goToSeeTheTutorial.cleanup();

		statsText.cleanup();
		statsObjects.cleanup();
		statsEnter.cleanup();
		statsEnterController.cleanup();
		statsExit.cleanup();
		statsExitController.cleanup();

		statsValueDoors3.cleanup();
		statsValueDoors4.cleanup();
		statsValueDoors5.cleanup();
		statsValueKeys.cleanup();
		statsValueFruit0.cleanup();
		statsValueFruit1.cleanup();
		statsValueLevers1.cleanup();
		statsValueLevers2.cleanup();
		statsValueLevers3.cleanup();
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {

		objectsCleanup();

		descriptorsCleanup();

		localPipelineCleanup();

		descriptorLayoutsCleanup();
	}





	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		drawSingleInstanceInGlobal(commandBuffer, currentImage, P1, DS_global, 0, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, mainCharacter, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, floorObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, ceilingObject, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, wallEastObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, wallNorthObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, wallSouthObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, wallWestObject, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, goldKeyObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, goldKeyHoleObject, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, copperKeyObject, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, copperKeyHoleObject, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, doorBorders, 1, 1);

		drawMultipleInstance(commandBuffer, currentImage, P1, doors, 1, 1);

		drawMultipleInstance(commandBuffer, currentImage, P1, levers, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, powerUp, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, powerUpBase, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, winText, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, winPlayAgain, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, winPlayAgainController, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, torch, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, tutorial, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, interaction, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, movement, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, visual, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, windowTutorial, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, restart, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, tutorialAgain, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, next, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, end, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, skip, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, jumpObject, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, interactionController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, movementController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, visualController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, restartController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, tutorialAgainController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, nextController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, endController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, skipController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, jumpController, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, welcomeTextInTheGame, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, startPlayTheGame, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, goToSeeTheTutorial, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, statsText, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsObjects, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsEnter, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsEnterController, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsExit, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsExitController, 1, 1);

		drawSingleInstance(commandBuffer, currentImage, P1, statsValueDoors3, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueDoors4, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueDoors5, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueKeys, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueFruit0, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueFruit1, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueLevers1, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueLevers2, 1, 1);
		drawSingleInstance(commandBuffer, currentImage, P1, statsValueLevers3, 1, 1);
	}





	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		//Initializae controller keys
		initializeInputKeys();

		//Handle mapping of gamepad
		/*if (controllerPlugged == 1) {
			loadFileContents();
		}*/

		//Manage menu input and logic
		if (menuOpen) {
			manageMenu();
		}

		//Manage tutorial input and logic
		if ((firstTimeDoingTheTutorial || doneTutorialAgain) && !menuOpen) {
			manageTutorial();
		}

		//Resize to full screen
		updateWindow();

		//Manage the logic to start again the tutorial
		seeTutorialAgain();

		//Manage the logic to restart the game
		if (!firstTimeDoingTheTutorial && !doneTutorialAgain && !menuOpen) {
			restartTheGame();
		}

		CharacterPos = updateCameraPosition(window);

		//Manage the interaction with the objects
		interactWithObjects();

		//Manage the winning condition
		checkWinning();

		GlobalUniformBufferObject gubo{};
		UniformBufferObject ubo{};

		void* data;
		gubo.view = CharacterPos;
		gubo.proj = glm::perspective(glm::radians(90.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 50.0f);
		gubo.proj[1][1] *= -1;
		//If we have already taken the torch the position of the light changes, otherwise it remains static (and it will not be present in the tutorial)
		if (doneTutorialAgain && !firstTimeDoingTheTutorial) {
			gubo.torchPos = torchPosStatic;
		}
		else {
			gubo.torchPos = torchPos;
		}
		gubo.eyePos = CamPos;



		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
			sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);





		//Character
		ubo.model = glm::mat4(1.0f);
		//-0.2f to let it touch it the ground
		ubo.model = glm::rotate(glm::translate(glm::mat4(1), pos + glm::vec3(0, -0.2f, 0)), lookYaw, glm::vec3(0, 1, 0)) * ubo.model;
		//ubo.model = glm::rotate(glm::rotate(glm::translate(glm::mat4(1), pos + glm::vec3(0, -0.2f, 0)), lookYaw, glm::vec3(0, 1, 0)), lookPitch, glm::vec3(1, 0, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 50.0f;
		updateObject(mainCharacter, ubo, currentImage);





		//Floor and ceiling
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(floorObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(ceilingObject, ubo, currentImage);


		//Walls
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(wallEastObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(wallNorthObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(wallSouthObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(wallWestObject, ubo, currentImage);





		//Gold key
		ubo.model = glm::mat4(1.0f);
		if (doorOpenOrNot[4] == 1) {
			//Make the key disappear from the screen
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, keyTakenOrNot[0] * 1.01f, 0)) * ubo.model;
		}
		else if (keyTakenOrNot[0] == 1) {
			//Make the key follow you
			ubo.model = glm::translate(glm::mat4(1), goldKeyPos) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 50.0f;
		updateObject(goldKeyObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 50.0f;
		updateObject(goldKeyHoleObject, ubo, currentImage);



		//Copper key
		ubo.model = glm::mat4(1.0f);
		if (doorOpenOrNot[1] == 1) {
			//Make the key disappear from the screen
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, keyTakenOrNot[1] * 1.01f, 0)) * ubo.model;
		}
		else if (keyTakenOrNot[1] == 1) {
			//Make the key follow you
			ubo.model = glm::translate(glm::mat4(1), copperKeyPos) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 50.0f;
		updateObject(copperKeyObject, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 50.0f;
		updateObject(copperKeyHoleObject, ubo, currentImage);






		//Door borders
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateObject(doorBorders, ubo, currentImage);


		//Doors
		ubo.model = glm::mat4(1.0f);
		//Logic that manage the opening and closure of a door (when a lever is involved), otherwise only opening
		if (doorOpenOrNot[0] == 1 && slowlyOpenDoors[0] < 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[0] * slowlyOpenDoors[0], 0)) * ubo.model;
			slowlyOpenDoors[0] += 0.002f;
		}
		else if (doorOpenOrNot[0] == 1 && slowlyOpenDoors[0] >= 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[0] * slowlyOpenDoors[0], 0)) * ubo.model;
		}
		else if (doorOpenOrNot[0] == 0 && slowlyOpenDoors[0] > 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, slowlyOpenDoors[0], 0)) * ubo.model;
			slowlyOpenDoors[0] -= 0.002f;
		}
		else if (doorOpenOrNot[0] == 0 && slowlyOpenDoors[0] <= 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[0] * 0, 0)) * ubo.model;
		}
		if (userCouldPassThroughDoors[0] == 0 && slowlyOpenDoors[0] >= 0.95f) {
			userCouldPassThroughDoors[0] = 1;
		}
		if (userCouldPassThroughDoors[0] == 1 && slowlyOpenDoors[0] < 0.9f) {
			userCouldPassThroughDoors[0] = 0;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 300.0f;
		updateOneInstanceOfObject(doors, ubo, currentImage, 0);

		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(3, 0, 5)) * ubo.model;
		if (doorOpenOrNot[1] == 1 && slowlyOpenDoors[1] < 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[1] * slowlyOpenDoors[1], 0)) * ubo.model;
			slowlyOpenDoors[1] += 0.002f;
		}
		else if (slowlyOpenDoors[1] >= 1.01f) {
			userCouldPassThroughDoors[1] = 1;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[1] * slowlyOpenDoors[1], 0)) * ubo.model;
		}
		if (userCouldPassThroughDoors[1] == 0 && slowlyOpenDoors[1] >= 0.95f) {
			userCouldPassThroughDoors[1] = 1;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 300.0f;
		updateOneInstanceOfObject(doors, ubo, currentImage, 1);

		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(5, 0, 0)) * ubo.model;
		//Rotate on the correct axis
		ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(9.0f, 0.0f, 3.0f))) * ubo.model;
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(9.0f, 0.0f, 3.0f)) * ubo.model;
		if (doorOpenOrNot[2] == 1 && slowlyOpenDoors[2] < 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[2] * slowlyOpenDoors[2], 0)) * ubo.model;
			slowlyOpenDoors[2] += 0.002f;
		}
		else if (doorOpenOrNot[2] == 1 && slowlyOpenDoors[2] >= 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[2] * slowlyOpenDoors[2], 0)) * ubo.model;
		}
		else if (doorOpenOrNot[2] == 0 && slowlyOpenDoors[2] > 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, slowlyOpenDoors[2], 0)) * ubo.model;
			slowlyOpenDoors[2] -= 0.002f;
		}
		else if (doorOpenOrNot[2] == 0 && slowlyOpenDoors[2] <= 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[2] * 0, 0)) * ubo.model;
		}
		if (userCouldPassThroughDoors[2] == 0 && slowlyOpenDoors[2] >= 0.95f) {
			userCouldPassThroughDoors[2] = 1;
		}
		if (userCouldPassThroughDoors[2] == 1 && slowlyOpenDoors[2] < 0.9f) {
			userCouldPassThroughDoors[2] = 0;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 300.0f;
		updateOneInstanceOfObject(doors, ubo, currentImage, 2);

		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5)) * ubo.model;
		//Rotate on the correct axis
		ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, -2.0f))) * ubo.model;
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, -2.0f)) * ubo.model;
		if (doorOpenOrNot[3] == 1 && slowlyOpenDoors[3] < 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[3] * slowlyOpenDoors[3], 0)) * ubo.model;
			slowlyOpenDoors[3] += 0.002f;
		}
		else if (doorOpenOrNot[3] == 1 && slowlyOpenDoors[3] >= 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[3] * slowlyOpenDoors[3], 0)) * ubo.model;
		}
		else if (doorOpenOrNot[3] == 0 && slowlyOpenDoors[3] > 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, slowlyOpenDoors[3], 0)) * ubo.model;
			slowlyOpenDoors[3] -= 0.002f;
		}
		else if (doorOpenOrNot[3] == 0 && slowlyOpenDoors[3] <= 0.0f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[3] * 0, 0)) * ubo.model;
		}
		if (userCouldPassThroughDoors[3] == 0 && slowlyOpenDoors[3] >= 0.95f) {
			userCouldPassThroughDoors[3] = 1;
		}
		if (userCouldPassThroughDoors[3] == 1 && slowlyOpenDoors[3] < 0.9f) {
			userCouldPassThroughDoors[3] = 0;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 300.0f;
		updateOneInstanceOfObject(doors, ubo, currentImage, 3);

		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(8, 0, 1)) * ubo.model;
		//Rotate on the correct axis
		ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, 0.0f, 4.0f))) * ubo.model;
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, 0.0f, 4.0f)) * ubo.model;
		if (doorOpenOrNot[4] == 1 && slowlyOpenDoors[4] < 1.01f) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[4] * slowlyOpenDoors[4], 0)) * ubo.model;
			slowlyOpenDoors[4] += 0.002f;
		}
		else if (slowlyOpenDoors[4] >= 1.01f) {
			userCouldPassThroughDoors[4] = 1;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, doorOpenOrNot[4] * slowlyOpenDoors[4], 0)) * ubo.model;
		}
		if (userCouldPassThroughDoors[4] == 0 && slowlyOpenDoors[4] >= 0.95f) {
			userCouldPassThroughDoors[4] = 1;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 300.0f;
		updateOneInstanceOfObject(doors, ubo, currentImage, 4);





		//Levers
		ubo.model = glm::mat4(1.0f);
		if (leverUsedOrNot[0] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 2.2f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), leverUsedOrNot[0] * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 2.2f)) * ubo.model;
		}
		else if (leverUsedOrNot[0] == 0 && doorOpenOrNot[0] == 0 && doorAlreadyOpened[0] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 2.2f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.5f, 2.2f)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateOneInstanceOfObject(levers, ubo, currentImage, 0);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(5.8f, 0, 1.3f)) * ubo.model;
		//Rotate on the correct axis
		ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f))) * ubo.model;
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f)) * ubo.model;
		if (leverUsedOrNot[1] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), leverUsedOrNot[1] * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f)) * ubo.model;
		}
		else if (leverUsedOrNot[1] == 0 && doorOpenOrNot[2] == 0 && doorAlreadyOpened[2] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(8.3f, 0.5f, 3.5f)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateOneInstanceOfObject(levers, ubo, currentImage, 1);
		ubo.model = glm::mat4(1.0f);
		//Rotate on the correct axis
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0, -3.7f)) * ubo.model;
		ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f))) * ubo.model;
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f)) * ubo.model;
		if (leverUsedOrNot[2] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), leverUsedOrNot[2] * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f)) * ubo.model;
		}
		else if (leverUsedOrNot[2] == 0 && doorOpenOrNot[3] == 0 && doorAlreadyOpened[3] == 1) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.3f, 0.5f, -1.5f)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 200.0f;
		updateOneInstanceOfObject(levers, ubo, currentImage, 2);





		//Power up
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, powerUpTakenOrNot * 10.0f, 0)) * ubo.model;
		//Rotate on the correct axis
		if (powerUpTakenOrNot == 0) {
			ubo.model = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.4f, 12.0f))) * ubo.model;
			ubo.model = glm::rotate(glm::mat4(1.0f), rotatingPowerUp * glm::radians(0.10f), glm::vec3(0.0f, 1.0f, 0.0f)) * ubo.model;
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.4f, 12.0f)) * ubo.model;
			rotatingPowerUp++;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 20.0f;
		updateObject(powerUp, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 20.0f;
		updateObject(powerUpBase, ubo, currentImage);





		//Win
		if (win && enterInTheStats != 0) {
			ubo.model = glm::mat4(1.0f);
		}
		else {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(winText, ubo, currentImage);

		if (win) {
			if (controllerPlugged == 1) {
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(winPlayAgain, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(winPlayAgainController, ubo, currentImage);
			}
			else {
				ubo.model = glm::mat4(1.0f);
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(winPlayAgain, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(winPlayAgainController, ubo, currentImage);
			}
		}
		else {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(winPlayAgain, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(winPlayAgainController, ubo, currentImage);
		}





		//Torch
		ubo.model = glm::mat4(1.0f);
		if (torchTaken) {
			ubo.model = glm::translate(glm::mat4(1), objectTorchPos + glm::vec3(0.0f, -0.45f, 0.0f)) * ubo.model;
			//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -0.8f, 4.2f)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		//Not place the object in the world but in the view
		ubo.roughness = 50.0f;
		updateObject(torch, ubo, currentImage);





		//Tutorial - is different when a gamepad is plugged or not
		if (controllerPlugged == 1 && keyboardInput) {
			controllerInput = true;
			keyboardInput = false;
			switchInput = true;
		}
		else if (controllerPlugged == 0 && controllerInput) {
			controllerInput = false;
			keyboardInput = true;
			switchInput = true;
		}
		if (switchInput && controllerInput) {
			switchInput = false;
			tutorialElementsController = tutorialElements;
			tutorialNextElementsController = tutorialNextElements;
			skipElementController = skipElement;
			tutorialElements = { 1,1,1,1,1,1,1,1,1 };
			tutorialNextElements = { 1,1 };
			skipElement = 1;
		}
		else if (switchInput && keyboardInput) {
			switchInput = false;
			tutorialElements = tutorialElementsController;
			tutorialNextElements = tutorialNextElementsController;
			skipElement = skipElementController;
			tutorialElementsController = { 1,1,1,1,1,1,1,1,1 };
			tutorialNextElementsController = { 1,1 };
			skipElementController = 1;
		}

		if (controllerPlugged == 1) {
			ubo.model = glm::mat4(1.0f);
			if (menuOpen) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[0] * 100.0f, 0)) * ubo.model;
			}
			else if (tutorialElementsController[0] == 1 && moveSlowlyTutorialController[0] > -20.01f) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorialController[0], 0, 0)) * ubo.model;
				moveSlowlyTutorial[0] -= 0.075f;
				moveSlowlyTutorialController[0] -= 0.075f;
				seeTheNextElementOfTutorial = false;
			}
			else if (tutorialElementsController[0] == 1 && moveSlowlyTutorialController[0] <= -20.01f) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorialController[0], 0, 0)) * ubo.model;
			}
			//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[0] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(tutorial, ubo, currentImage);
			ubo.model = manageTutorialTexts(1, true);
			//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[1] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(windowTutorial, ubo, currentImage);
		}
		else {
			ubo.model = glm::mat4(1.0f);
			if (menuOpen) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[0] * 100.0f, 0)) * ubo.model;
			}
			else if (tutorialElements[0] == 1 && moveSlowlyTutorial[0] > -20.01f) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorial[0], 0, 0)) * ubo.model;
				moveSlowlyTutorial[0] -= 0.075f;
				moveSlowlyTutorialController[0] -= 0.075f;
				seeTheNextElementOfTutorial = false;
			}
			else if (tutorialElements[0] == 1 && moveSlowlyTutorial[0] <= -20.01f) {
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorial[0], 0, 0)) * ubo.model;
			}
			//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[0] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(tutorial, ubo, currentImage);
			ubo.model = manageTutorialTexts(1, false);
			//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[1] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(windowTutorial, ubo, currentImage);
		}
		ubo.model = manageTutorialTexts(2, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[2] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(visualController, ubo, currentImage);
		ubo.model = manageTutorialTexts(3, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[3] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(movementController, ubo, currentImage);
		ubo.model = manageTutorialTexts(4, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[4] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(jumpController, ubo, currentImage);
		ubo.model = manageTutorialTexts(5, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[5] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(interactionController, ubo, currentImage);
		ubo.model = manageTutorialTexts(6, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[6] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(restartController, ubo, currentImage);
		ubo.model = manageTutorialTexts(7, true);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElementsController[7] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(tutorialAgainController, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialNextElementsController[0] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(nextController, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialNextElementsController[1] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(endController, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, skipElementController * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(skipController, ubo, currentImage);

		ubo.model = manageTutorialTexts(2, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[2] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(visual, ubo, currentImage);
		ubo.model = manageTutorialTexts(3, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[3] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(movement, ubo, currentImage);
		ubo.model = manageTutorialTexts(4, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[4] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(jumpObject, ubo, currentImage);
		ubo.model = manageTutorialTexts(5, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[5] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(interaction, ubo, currentImage);
		ubo.model = manageTutorialTexts(6, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[6] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(restart, ubo, currentImage);
		ubo.model = manageTutorialTexts(7, false);
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialElements[7] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(tutorialAgain, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialNextElements[0] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(next, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, tutorialNextElements[1] * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(end, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, skipElement * 100.0f, 0)) * ubo.model;
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(skip, ubo, currentImage);





		//Menu
		ubo.model = glm::mat4(1.0f);
		if (!menuOpen) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(welcomeTextInTheGame, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		if (!menuOpen) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(goToSeeTheTutorial, ubo, currentImage);
		ubo.model = glm::mat4(1.0f);
		if (!menuOpen) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
		}
		ubo.normal = glm::inverse(glm::transpose(ubo.model));
		ubo.roughness = 500.0f;
		updateObject(startPlayTheGame, ubo, currentImage);





		//Stats
		if (win) {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, enterInTheStats * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsText, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, enterInTheStats * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsObjects, ubo, currentImage);
			if (controllerPlugged == 1) {
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsEnter, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, (1 - enterInTheStats) * 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsEnterController, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsExit, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, enterInTheStats * 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsExitController, ubo, currentImage);
			}
			else {
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, (1 - enterInTheStats) * 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsEnter, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsEnterController, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, enterInTheStats * 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsExit, ubo, currentImage);
				ubo.model = glm::mat4(1.0f);
				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
				ubo.normal = glm::inverse(glm::transpose(ubo.model));
				ubo.roughness = 500.0f;
				updateObject(statsExitController, ubo, currentImage);
			}
		}
		else {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsText, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsObjects, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsEnter, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsEnterController, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsExit, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsExitController, ubo, currentImage);
		}

		if (win && enterInTheStats != 1) {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForDoorsAtTheEnd[0] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors3, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForDoorsAtTheEnd[1] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors4, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForDoorsAtTheEnd[2] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors5, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForKeysAtTheEnd * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueKeys, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForFruitAtTheEnd[0] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueFruit0, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForFruitAtTheEnd[1] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueFruit1, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForLeversAtTheEnd[0] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers1, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForLeversAtTheEnd[1] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers2, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, statsForLeversAtTheEnd[2] * 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers3, ubo, currentImage);
		}
		else if (enterInTheStats == 1) {
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors3, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors4, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueDoors5, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueKeys, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueFruit0, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueFruit1, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers1, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers2, ubo, currentImage);
			ubo.model = glm::mat4(1.0f);
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * ubo.model;
			ubo.normal = glm::inverse(glm::transpose(ubo.model));
			ubo.roughness = 500.0f;
			updateObject(statsValueLevers3, ubo, currentImage);
		}
	}









	/// <summary>
	/// Handle the transition of the texts.
	/// </summary>
	/// <param name="numberOfText"></param>
	/// <param name="controllerLogic"></param>
	/// <returns></returns>
	glm::mat4 manageTutorialTexts(int numberOfText, bool controllerLogic) {
		glm::mat4 model = glm::mat4(1.0f);
		if (controllerLogic) {
			if (menuOpen || handlePreviousPosition(numberOfText, controllerLogic) || controllerPlugged != 1) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * model;
			}
			else if (tutorialElementsController[numberOfText] == 0 && moveSlowlyTutorialController[numberOfText] > 0.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorialController[numberOfText], 0, 0)) * model;
				moveSlowlyTutorial[numberOfText] -= 0.075f;
				moveSlowlyTutorialController[numberOfText] -= 0.075f;
			}
			else if (tutorialElementsController[numberOfText] == 0 && moveSlowlyTutorialController[numberOfText] <= 0.01f) {
				model = glm::mat4(1.0f);
				seeTheNextElementOfTutorial = true;
			}
			else if (tutorialElementsController[numberOfText] == 1 && moveSlowlyTutorialController[numberOfText] > -20.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(tutorialElementsController[numberOfText] * moveSlowlyTutorialController[numberOfText], 0, 0)) * model;
				moveSlowlyTutorial[numberOfText] -= 0.075f;
				moveSlowlyTutorialController[numberOfText] -= 0.075f;
				seeTheNextElementOfTutorial = false;
			}
			else if (moveSlowlyTutorialController[numberOfText] <= -20.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(tutorialElementsController[numberOfText] * moveSlowlyTutorialController[numberOfText], 0, 0)) * model;
			}
		}
		else {
			if (menuOpen || handlePreviousPosition(numberOfText, controllerLogic) || controllerPlugged == 1) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 100.0f, 0)) * model;
			}
			else if (tutorialElements[numberOfText] == 0 && moveSlowlyTutorial[numberOfText] > 0.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(moveSlowlyTutorial[numberOfText], 0, 0)) * model;
				moveSlowlyTutorial[numberOfText] -= 0.075f;
				moveSlowlyTutorialController[numberOfText] -= 0.075f;
			}
			else if (tutorialElements[numberOfText] == 0 && moveSlowlyTutorial[numberOfText] <= 0.01f) {
				model = glm::mat4(1.0f);
				seeTheNextElementOfTutorial = true;
			}
			else if (tutorialElements[numberOfText] == 1 && moveSlowlyTutorial[numberOfText] > -20.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(tutorialElements[numberOfText] * moveSlowlyTutorial[numberOfText], 0, 0)) * model;
				moveSlowlyTutorial[numberOfText] -= 0.075f;
				moveSlowlyTutorialController[numberOfText] -= 0.075f;
				seeTheNextElementOfTutorial = false;
			}
			else if (moveSlowlyTutorial[numberOfText] <= -20.01f) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(tutorialElements[numberOfText] * moveSlowlyTutorial[numberOfText], 0, 0)) * model;
			}
		}
		return model;
	}

	/// <summary>
	/// Handle the logic of the already passed tutorial texts.
	/// </summary>
	/// <param name="numberOfText"></param>
	/// <param name="controllerLogic"></param>
	/// <returns></returns>
	bool handlePreviousPosition(int numberOfText, bool controllerLogic) {
		if (controllerLogic) {
			for (int i = 0; i < tutorialElementsController.size(); i++) {
				if (i == numberOfText) {
					break;
				}
				if (tutorialElementsController[i] == 0) {
					return true;
				}
			}
		}
		else {
			for (int i = 0; i < tutorialElements.size(); i++) {
				if (i == numberOfText) {
					break;
				}
				if (tutorialElements[i] == 0) {
					return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// Initialize controller input keys
	/// </summary>
	void initializeInputKeys() {
		controllerPlugged = glfwJoystickPresent(GLFW_JOYSTICK_1);

		axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);

		//std::cout << "\nLeft stick X Axis: " << axes[0];
		//std::cout << "\nLeft stick Y Axis: " << axes[1];
		//std::cout << "\nRight stick X Axis: " << axes[2];
		//std::cout << "\nRight stick Y Axis: " << axes[3];
		//std::cout << "\nL2: " << axes[4];
		//std::cout << "\nR2: " << axes[5];

		buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
		/*if (buttons[0] == GLFW_PRESS) {
			//Square
			//std::cout << buttons[0];
		}
		if (buttons[1] == GLFW_PRESS) {
			//X
			//std::cout << buttons[1];
		}*/
		/*if (buttons[2] == GLFW_PRESS) {
			//Circle
			//std::cout << buttons[2];
		}
		if (buttons[3] == GLFW_PRESS) {
			//Triangle
			//std::cout << buttons[3];
		}*/
	}

	/// <summary>
	/// Manage the menu and the movement to the menu and to the game
	/// </summary>
	void manageMenu() {
		lookYaw = 0;
		lookPitch = 0;
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		//Handle position of the mouse with the controller
		if (controllerPlugged == 1 && xpos >= 0 && xpos <= width && ypos >= 0 && ypos <= height) {
			//glfwSetCursorPos(window, xpos + axes[2], ypos + axes[3]);
			glfwSetCursorPos(window, xpos + axes[2], height / 2);
		}
		int state;
		if (controllerPlugged == 1) {
			state = buttons[1];
		}
		else {
			state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		}
		if (xpos < width / 2) {
			if (state == GLFW_PRESS)
			{
				menuOpen = false;
				enterTheGame = true;
			}
		}
		else {
			if (state == GLFW_PRESS)
			{
				menuOpen = false;
				enterTheTutorial = true;
			}
		}
		if (enterTheGame) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			initialBackgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			//Tutorial
			firstTimeDoingTheTutorial = false;
			doneTutorialAgain = false;
			skipElement = 1;
			tutorialElements = { 1,1,1,1,1,1,1,1 };
			tutorialNextElements = { 1,1 };
			skipElementController = 1;
			tutorialElementsController = { 1,1,1,1,1,1,1,1 };
			tutorialNextElementsController = { 1,1 };
			//Game
			pos = glm::vec3(0.0f, 0.5f, 0.0f);
			oldPos = glm::vec3(0.0f, 0.5f, 0.0f);
			lookYaw = glm::radians(-45.0f);
			lookPitch = 0.0f;
		}
		else if (enterTheTutorial) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			initialBackgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			//Tutorial
			if (controllerPlugged == 1) {
				skipElementController = 0;
				tutorialElementsController = { 0,1,1,1,1,1,1,1 };
				tutorialNextElementsController = { 0,1 };
				skipElement = 1;
				tutorialElements = { 1,1,1,1,1,1,1,1 };
				tutorialNextElements = { 1,1 };
			}
			else {
				skipElementController = 1;
				tutorialElementsController = { 1,1,1,1,1,1,1,1 };
				tutorialNextElementsController = { 1,1 };
				skipElement = 0;
				tutorialElements = { 0,1,1,1,1,1,1,1 };
				tutorialNextElements = { 0,1 };
			}
		}
	}

	/// <summary>
	/// Manage the next page of the tutorial and the skip of the whole tutorial
	/// </summary>
	void manageTutorial() {
		if (controllerPlugged == 1) {
			//R1
			if (buttons[2] == GLFW_PRESS || buttons[5] == GLFW_PRESS)
			{
				stateTutorial = true;
			}
			else
			{
				stateTutorial = false;
				firstTutorial = true;
			}
			if (buttons[2] == GLFW_PRESS && firstTutorial && seeTheNextElementOfTutorial) {
				if (firstTutorial) {
					firstTutorial = false;
				}
				tutorialLogic(tutorialElementsController, tutorialNextElementsController, skipElementController, true);
			}
			//R1
			else if (buttons[5] == GLFW_PRESS && firstTutorial && seeTheNextElementOfTutorial) {
				if (firstTutorial) {
					firstTutorial = false;
				}
				if (doneTutorialAgain) {
					doneTutorialAgain = false;
					pos = savePos;
					oldPos = savePos;
					lookPitch = lookPitchSaved;
					lookYaw = lookYawSaved;
				}
				else {
					pos = glm::vec3(0.0f, 0.5f, 0.0f);
					oldPos = glm::vec3(0.0f, 0.5f, 0.0f);
					lookYaw = glm::radians(-45.0f);
					lookPitch = 0.0f;
				}
				firstTimeDoingTheTutorial = false;
				skipElementController = 1;
				tutorialElementsController = { 1,1,1,1,1,1,1,1 };
				tutorialNextElementsController = { 1,1 };
			}
		}
		else {
			int stateTutorialInt = glfwGetKey(window, GLFW_KEY_N);
			int stateTutorialInt2 = glfwGetKey(window, GLFW_KEY_ESCAPE);
			if (stateTutorialInt == GLFW_PRESS || stateTutorialInt2 == GLFW_PRESS)
			{
				stateTutorial = true;
			}
			else
			{
				stateTutorial = false;
				firstTutorial = true;
			}
			if (glfwGetKey(window, GLFW_KEY_N) && firstTutorial && seeTheNextElementOfTutorial) {
				if (firstTutorial) {
					firstTutorial = false;
				}
				tutorialLogic(tutorialElements, tutorialNextElements, skipElement, false);
			}
			else if (glfwGetKey(window, GLFW_KEY_ESCAPE) && firstTutorial && seeTheNextElementOfTutorial) {
				if (firstTutorial) {
					firstTutorial = false;
				}
				if (doneTutorialAgain) {
					doneTutorialAgain = false;
					pos = savePos;
					oldPos = savePos;
					lookPitch = lookPitchSaved;
					lookYaw = lookYawSaved;
				}
				else {
					pos = glm::vec3(0.0f, 0.5f, 0.0f);
					oldPos = glm::vec3(0.0f, 0.5f, 0.0f);
					lookYaw = glm::radians(-45.0f);
					lookPitch = 0.0f;
				}
				firstTimeDoingTheTutorial = false;
				skipElement = 1;
				tutorialElements = { 1,1,1,1,1,1,1,1 };
				tutorialNextElements = { 1,1 };
			}
		}
	}

	/// <summary>
	/// Manage tutorial logic.
	/// </summary>
	/// <param name="tutorialElementsFunc"></param>
	/// <param name="tutorialNextElementsFunc"></param>
	/// <param name="skipElementFunc"></param>
	/// <param name="isControllerPlugged"></param>
	void tutorialLogic(std::vector<int> tutorialElementsFunc, std::vector<int> tutorialNextElementsFunc, int skipElementFunc, bool isControllerPlugged) {
		if (tutorialElementsFunc[0] == 0) {
			tutorialElementsFunc[0] = 1;
			tutorialElementsFunc[1] = 0;
		}
		else if (tutorialElementsFunc[1] == 0) {
			tutorialElementsFunc[1] = 1;
			tutorialElementsFunc[2] = 0;
		}
		else if (tutorialElementsFunc[2] == 0) {
			tutorialElementsFunc[2] = 1;
			tutorialElementsFunc[3] = 0;
		}
		else if (tutorialElementsFunc[3] == 0) {
			tutorialElementsFunc[3] = 1;
			tutorialElementsFunc[4] = 0;
		}
		else if (tutorialElementsFunc[4] == 0) {
			tutorialElementsFunc[4] = 1;
			tutorialElementsFunc[5] = 0;
		}
		else if (tutorialElementsFunc[5] == 0) {
			tutorialElementsFunc[5] = 1;
			tutorialElementsFunc[6] = 0;
		}
		else if (tutorialElementsFunc[6] == 0) {
			tutorialElementsFunc[6] = 1;
			tutorialElementsFunc[7] = 0;
			tutorialNextElementsFunc[0] = 1;
			tutorialNextElementsFunc[1] = 0;
			skipElementFunc = 1;
		}
		else if (tutorialElementsFunc[7] == 0) {
			tutorialElementsFunc[7] = 1;
			tutorialNextElementsFunc[1] = 1;
			//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			if (doneTutorialAgain) {
				doneTutorialAgain = false;
				pos = savePos;
				oldPos = savePos;
				lookPitch = lookPitchSaved;
				lookYaw = lookYawSaved;
			}
			else {
				pos = glm::vec3(0.0f, 0.5f, 0.0f);
				oldPos = glm::vec3(0.0f, 0.5f, 0.0f);
				lookYaw = glm::radians(-45.0f);
				lookPitch = 0.0f;
			}
			firstTimeDoingTheTutorial = false;
		}
		if (isControllerPlugged) {
			tutorialElementsController = tutorialElementsFunc;
			tutorialNextElementsController = tutorialNextElementsFunc;
			skipElementController = skipElementFunc;
		}
		else {
			tutorialElements = tutorialElementsFunc;
			tutorialNextElements = tutorialNextElementsFunc;
			skipElement = skipElementFunc;
		}
	}

	/// <summary>
	/// To see the tutorial again.
	/// </summary>
	void seeTutorialAgain() {
		if (!win) {
			if (controllerPlugged == 1) {
				if (buttons[9] == GLFW_PRESS)
				{
					stateTutorialAgain = true;
				}
				else
				{
					stateTutorialAgain = false;
					firstTutorialAgain = true;
				}
				if (buttons[9] == GLFW_PRESS && firstTutorialAgain) {
					if (firstTutorialAgain) {
						firstTutorialAgain = false;
					}
					tutorialAgainLogic(true);
				}
			}
			else {
				int stateTutorialAgainInt = glfwGetKey(window, GLFW_KEY_T);
				if (stateTutorialAgainInt == GLFW_PRESS)
				{
					stateTutorialAgain = true;
				}
				else
				{
					stateTutorialAgain = false;
					firstTutorialAgain = true;
				}
				if (glfwGetKey(window, GLFW_KEY_T) && firstTutorialAgain) {
					if (firstTutorialAgain) {
						firstTutorialAgain = false;
					}
					tutorialAgainLogic(false);
				}
			}
		}
	}

	/// <summary>
	/// Manage the logic to see the tutorial again.
	/// </summary>
	/// <param name="isControllerPlugged"></param>
	void tutorialAgainLogic(bool isControllerPlugged) {
		doneTutorialAgain = true;
		if (isControllerPlugged) {
			tutorialElementsController = { 0,1,1,1,1,1,1,1 };
			tutorialNextElementsController = { 0,1 };
			skipElementController = 0;
		}
		else {
			tutorialElements = { 0,1,1,1,1,1,1,1 };
			tutorialNextElements = { 0,1 };
			skipElement = 0;
		}
		moveSlowlyTutorial = { 0.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f };
		moveSlowlyTutorialController = { 0.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f,20.0f };
		seeTheNextElementOfTutorial = true;
		if (pos.z <= -450.0f) {//-495.0f) {
			if (firstTimeDoingTheTutorial) {
				savePos = glm::vec3(0.0f, 0.5f, 0.0f);
				lookYawSaved = glm::radians(-45.0f);
				lookPitchSaved = 0.0f;
				torchPos = torchPosStatic;
				objectTorchPos = torchPosStatic;
			}
		}
		else {
			savePos = pos;
			lookPitchSaved = lookPitch;
			lookYawSaved = lookYaw;
		}
		pos = glm::vec3(500.0f, 199.5f, -495.0f);
		oldPos = glm::vec3(500.0f, 199.5f, -495.0f);
		lookYaw = 0.0f;
		lookPitch = 0.0f;
	}

	void restartTheGame() {
		if (controllerPlugged == 1) {
			//Share
			if (buttons[8] == GLFW_PRESS) {
				resetAll();
			}
		}
		else {
			if (glfwGetKey(window, GLFW_KEY_P)) {
				resetAll();
			}
		}
	}

	/// <summary>
	/// Reset all parameter to start a new game.
	/// </summary>
	void resetAll() {
		pos = glm::vec3(0.0f, 0.5f, 0.0f);
		oldPos = glm::vec3(0.0f, 0.5f, 0.0f);
		lookYaw = glm::radians(-45.0f);
		lookPitch = 0.0f;
		lookYawSaved = glm::radians(-45.0f);
		lookPitchSaved = 0.0f;
		lookRoll = 0.0f;

		win = false;
		firstWin = false;
		jump = false;
		jumpDown = false;

		CamPos = glm::vec3(0);
		CharacterPos = glm::mat4(0);


		//Environment
		doorOpenOrNot = { 0,0,0,0,0 };
		doorAlreadyOpened = { 0,0,0,0,0 };
		userCouldPassThroughDoors = { 0,0,0,0,0 };
		//Gold and Copper key
		doorCouldBeOpened = { 0,0 };
		leverCouldBeUsed = { 0,0,0 };
		leverUsedOrNot = { 0,0,0 };
		keyCouldBeTaken = { 0,0 };
		keyTakenOrNot = { 0,0 };
		powerUpTakenOrNot = 0.0f;

		rotatingPowerUp = 0;
		slowlyOpenDoors = { 0.0f,0.0f,0.0f,0.0f,0.0f };

		torchPos = torchPosStatic;
		objectTorchPos = torchPosStatic;
		torchCouldBeTaken = false;
		torchTaken = false;

		goldKeyPos = glm::vec3(0);
		copperKeyPos = glm::vec3(0);

		//Controller
		if (controllerPlugged == 1) {
			controllerInput = true;
			keyboardInput = false;
		}
		else {
			controllerInput = false;
			keyboardInput = true;
		}
		switchInput = false;

		//Stats
		enterInTheStats = 1;
		statsForDoorsAtTheEnd = { 1,1,1 };
		statsForKeysAtTheEnd = 1;
		statsForFruitAtTheEnd = { 1,1 };
		statsForLeversAtTheEnd = { 1,1,1 };
	}

	/// <summary>
	/// Update the camera position, movement and jumping.
	/// </summary>
	/// <param name="window">Window</param>
	/// <returns>The camera position.</returns>
	glm::mat4 updateCameraPosition(GLFWwindow* window) {
		const float ROT_SPEED = glm::radians(90.0f);
		//const float MOVE_SPEED = powerUpTakenOrNot + 1.2f;
		const float MOVE_SPEED = powerUpTakenOrNot + 2.0f;
		const float JUMP_SPEED = 2.0f;
		const float MOUSE_RES = 500.0f;

		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;

		oldPos = pos;


		if (controllerPlugged == 1) {
			lookYaw -= (round(axes[2] * 10.0) / 10.0) / 50.0;
			//lookPitch -= (round(axes[3] * 10.0) / 10.0) / 100.0;

			if ((round(axes[0] * 10.0) / 10.0) < -0.3f) {
				pos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			}
			if ((round(axes[0] * 10.0) / 10.0) > 0.3f) {
				pos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			}
			if ((round(axes[1] * 10.0) / 10.0) < -0.3f) {
				pos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			}
			if ((round(axes[1] * 10.0) / 10.0) > 0.3f) {
				pos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			}

			if (buttons[1] == GLFW_PRESS && !jump) {
				jump = true;
			}

			if (pos.y < 0.5f)
			{
				pos.y = 0.5f;
				jump = false;
				jumpDown = false;
			}

			if (pos.y >= 0.85) {
				jumpDown = true;
			}

			if (jump) {
				if (pos.y <= 0.85 && !jumpDown) {
					pos += JUMP_SPEED * glm::vec3(0, 0.7f, 0) * deltaT;
				}
				else {
					pos -= JUMP_SPEED * glm::vec3(0, 0.7f, 0) * deltaT;
				}
			}
		}
		else {
			//View
			if (glfwGetKey(window, GLFW_KEY_LEFT)) {
				lookYaw += deltaT * ROT_SPEED;
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
				lookYaw -= deltaT * ROT_SPEED;
			}
			if (glfwGetKey(window, GLFW_KEY_UP) && lookPitch < 1) {
				lookPitch += deltaT * ROT_SPEED;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) && lookPitch > -1) {
				lookPitch -= deltaT * ROT_SPEED;
			}

			//Movement
			if (glfwGetKey(window, GLFW_KEY_A)) {
				pos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
				pos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_W)) {
				pos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_S)) {
				pos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
					glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			}

			/*if (glfwGetKey(window, GLFW_KEY_SPACE)) {
				pos += MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
				pos -= MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
			}*/

			//Jump
			if (glfwGetKey(window, GLFW_KEY_SPACE) && !jump) {
				jump = true;
			}

			if (pos.y < 0.5f)
			{
				pos.y = 0.5f;
				jump = false;
				jumpDown = false;
			}

			if (pos.y >= 0.85) {
				jumpDown = true;
			}

			if (jump) {
				if (pos.y <= 0.85 && !jumpDown) {
					pos += JUMP_SPEED * glm::vec3(0, 0.7f, 0) * deltaT;
				}
				else {
					pos -= JUMP_SPEED * glm::vec3(0, 0.7f, 0) * deltaT;
				}
			}
		}



		if (!canStep(pos.x, pos.z)) {
			pos = oldPos;
		}



		CamPos = pos;



		if (doneTutorialAgain || firstTimeDoingTheTutorial || menuOpen || win) {
			lookYaw = 0.0f;
			lookPitch = 0.0f;
		}



		glm::mat4 out =
			glm::rotate(glm::mat4(1.0), -lookPitch, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0), -lookYaw, glm::vec3(0, 1, 0)) *
			glm::translate(glm::mat4(1.0), -pos);
		return out;
	}

	/// <summary>
	/// Move the character to the winning position
	/// </summary>
	void checkWinning() {
		if (win && firstWin) {
			firstWin = false;
			pos = glm::vec3(500.0f, 199.5f, -495.0f);
			oldPos = glm::vec3(500.0f, 199.5f, -495.0f);
			lookYaw = 0.0f;
			lookPitch = 0.0f;
			handleStats();
		}
		if (win) {
			if (controllerPlugged == 1) {
				//L1
				if (buttons[4] == GLFW_PRESS)
				{
					stateL1 = true;
				}
				else
				{
					stateL1 = false;
					firstL1 = true;
				}
				if (buttons[4] == GLFW_PRESS && firstL1) {
					if (firstL1) {
						firstL1 = false;
					}
					if (enterInTheStats == 1) {
						enterInTheStats--;
					}
					else {
						enterInTheStats++;
					}
				}
			}
			else {
				int stateXInt = glfwGetKey(window, GLFW_KEY_X);
				if (stateXInt == GLFW_PRESS)
				{
					stateX = true;
				}
				else
				{
					stateX = false;
					firstX = true;
				}
				if (glfwGetKey(window, GLFW_KEY_X) && firstX) {
					if (firstX) {
						firstX = false;
					}
					if (enterInTheStats == 1) {
						enterInTheStats--;
					}
					else {
						enterInTheStats++;
					}
				}
			}
		}
	}

	/// <summary>
	/// Handle the stats to be shown.
	/// </summary>
	void handleStats() {
		int temp = 0;
		for (int i = 0; i < 5; i++) {
			if (doorOpenOrNot[i] == 1) {
				temp++;
			}
		}
		temp = temp - 3;
		statsForDoorsAtTheEnd[temp] = 0;
		temp = 0;
		for (int i = 0; i < 3; i++) {
			if (leverUsedAtLeastOnce[i] == 1) {
				temp++;
			}
		}
		temp = temp - 1;
		statsForLeversAtTheEnd[temp] = 0;
		if (powerUpTakenOrNot != 0.0f) {
			statsForFruitAtTheEnd[1] = 0;
		}
		else {
			statsForFruitAtTheEnd[0] = 0;
		}
		statsForKeysAtTheEnd = 0;
	}

	/// <summary>
	/// When click F11, set full screen.
	/// </summary>
	void updateWindow() {
		int stateF11Int = glfwGetKey(window, GLFW_KEY_F11);
		if (stateF11Int == GLFW_PRESS)
		{
			stateF11 = true;
		}
		else
		{
			stateF11 = false;
			firstF11 = true;
		}
		if (glfwGetKey(window, GLFW_KEY_F11) && firstF11) {
			if (firstF11) {
				firstF11 = false;
			}
			int maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
			if (maximized == 0) {
				glfwMaximizeWindow(window);
			}
			else {
				glfwRestoreWindow(window);
			}
		}
	}

	/// <summary>
	/// Interact with all objects.
	/// </summary>
	void interactWithObjects() {
		manageTheKeys();

		openTheDoor();

		if (torchCouldBeTaken) {
			if (controllerPlugged == 1) {
				if (buttons[0] == GLFW_PRESS) {
					torchTaken = true;
				}
			}
			else {
				if (glfwGetKey(window, GLFW_KEY_E)) {
					torchTaken = true;
				}
			}
		}

		if (torchTaken) {
			torchPos = pos;
			objectTorchPos = manageTorchPositionObject(torchPosStatic);
		}
	}

	/// <summary>
	/// Manage the keys.
	/// </summary>
	void manageTheKeys() {
		if (controllerPlugged == 1) {
			if (buttons[0] == GLFW_PRESS) {
				if (keyCouldBeTaken[0] == 1) {
					keyTakenOrNot[0] = 1;
					keyCouldBeTaken[0] = 0;
				}
				if (keyCouldBeTaken[1] == 1) {
					keyTakenOrNot[1] = 1;
					keyCouldBeTaken[1] = 0;
				}
			}
		}
		else {
			if (glfwGetKey(window, GLFW_KEY_E)) {
				if (keyCouldBeTaken[0] == 1) {
					keyTakenOrNot[0] = 1;
					keyCouldBeTaken[0] = 0;
				}
				if (keyCouldBeTaken[1] == 1) {
					keyTakenOrNot[1] = 1;
					keyCouldBeTaken[1] = 0;
				}
			}
		}

		if (keyTakenOrNot[0] == 1) {
			goldKeyPos = manageKeyPosition(goldKeyPosStatic);
		}
		if (keyTakenOrNot[1] == 1) {
			copperKeyPos = manageKeyPosition(copperKeyPosStatic);
		}
	}

	glm::vec3 manageTorchPositionObject(glm::vec3 torchPosStatic) {
		glm::vec3 positionTempTorch = glm::vec3((pos.x - torchPosStatic.x), (pos.y - torchPosStatic.y) + 0.2f, (pos.z - torchPosStatic.z) + 0.08f);
		return positionTempTorch;
	}

	/// <summary>
	/// Manage the position of the key
	/// </summary>
	/// <param name="keyPosStatic">To recognize between gold key and copper key</param>
	/// <returns>The position of the key</returns>
	glm::vec3 manageKeyPosition(glm::vec3 keyPosStatic) {
		glm::vec3 positionTempKey = glm::vec3((pos.x - keyPosStatic.x), (pos.y - keyPosStatic.y) - 0.4f, (pos.z - keyPosStatic.z));
		return positionTempKey;
	}

	/// <summary>
	/// Check the interaction with the doors.
	/// </summary>
	void openTheDoor() {
		if (controllerPlugged == 1) {
			if (buttons[0] == GLFW_PRESS)
			{
				stateInteraction = true;
			}
			else
			{
				stateInteraction = false;
				firstInteraction = true;
			}
			if (buttons[0] == GLFW_PRESS && firstInteraction) {
				if (firstInteraction) {
					firstInteraction = false;
				}
				doorOpeningLogic();
			}
		}
		else {
			int stateInteractionInt = glfwGetKey(window, GLFW_KEY_E);
			if (stateInteractionInt == GLFW_PRESS)
			{
				stateInteraction = true;
			}
			else
			{
				stateInteraction = false;
				firstInteraction = true;
			}
			if (glfwGetKey(window, GLFW_KEY_E) && firstInteraction) {
				if (firstInteraction) {
					firstInteraction = false;
				}
				doorOpeningLogic();
			}
		}
	}

	/// <summary>
	/// Open the doors and use the levers.
	/// </summary>
	void doorOpeningLogic() {
		if (leverCouldBeUsed[2] == 1 && leverUsedOrNot[2] == 0) {
			doorOpenOrNot[3] = 1;
			leverUsedOrNot[2] = 1;
			doorAlreadyOpened[3] = 1;
			leverUsedAtLeastOnce[2] = 1;
		}
		else if (leverCouldBeUsed[2] == 1 && leverUsedOrNot[2] == 1) {
			doorOpenOrNot[3] = 0;
			leverUsedOrNot[2] = 0;
		}

		if (leverCouldBeUsed[1] == 1 && leverUsedOrNot[1] == 0) {
			doorOpenOrNot[2] = 1;
			leverUsedOrNot[1] = 1;
			doorAlreadyOpened[2] = 1;
			leverUsedAtLeastOnce[1] = 1;
		}
		else if (leverCouldBeUsed[1] == 1 && leverUsedOrNot[1] == 1) {
			doorOpenOrNot[2] = 0;
			leverUsedOrNot[1] = 0;
		}

		if (leverCouldBeUsed[0] == 1 && leverUsedOrNot[0] == 0) {
			doorOpenOrNot[0] = 1;
			leverUsedOrNot[0] = 1;
			doorAlreadyOpened[0] = 1;
			leverUsedAtLeastOnce[0] = 1;
		}
		else if (leverCouldBeUsed[0] == 1 && leverUsedOrNot[0] == 1) {
			doorOpenOrNot[0] = 0;
			leverUsedOrNot[0] = 0;
		}

		if (doorCouldBeOpened[0] == 1 && doorOpenOrNot[4] == 0) {
			doorOpenOrNot[4] = 1;
		}

		if (doorCouldBeOpened[1] == 1 && doorOpenOrNot[1] == 0) {
			doorOpenOrNot[1] = 1;
		}
	}

	void descriptorSetLayoutInit(DescriptorSetLayoutObject* descriptorSetLayoutObject) {
		std::vector<DescriptorSetLayoutBinding> elementOfDSL;
		elementOfDSL.resize(descriptorSetLayoutObject->set.size());
		for (int i = 0; i < descriptorSetLayoutObject->set.size(); i++) {
			elementOfDSL[i].binding = descriptorSetLayoutObject->set[i];
			elementOfDSL[i].type = descriptorSetLayoutObject->vkDescriptorType[i];
			elementOfDSL[i].flags = descriptorSetLayoutObject->vkShaderStageFlagBits[i];
		}
		descriptorSetLayoutObject->descriptorSetLayout.init(this, { elementOfDSL });
	}

	/// <summary>
	/// Get the required size.
	/// </summary>
	/// <param name="descriptorSetElementType">The type of descriptor</param>
	/// <param name="isAnObject">If is an object and needs a model.</param>
	/// <returns>The required size</returns>
	uint64_t getSizeForObject(DescriptorSetElementType descriptorSetElementType, bool isAnObject) {
		if (descriptorSetElementType == TEXTURE) {
			return 0;
		}
		else if (isAnObject) {
			return sizeof(UniformBufferObject);
		}
		else {
			return sizeof(GlobalUniformBufferObject);
		}
	}

	/// <summary>
	/// Return pointer of the texture.
	/// </summary>
	/// <param name="descriptorSetElementType">The type of descriptor</param>
	/// <param name="object">The object from which take the texture</param>
	/// <returns>The pointer of the texture</returns>
	Texture* getPointerOfTexture(DescriptorSetElementType descriptorSetElementType, Object* object) {
		if (descriptorSetElementType == TEXTURE) {
			return &object->texture;
		}
		else {
			return nullptr;
		}
	}

	void objectInit(Object* object, std::string modelPath, std::string texturePath, DescriptorSetLayout descriptorSetLayout, DescriptorSetLayoutObject descriptorSetLayoutObject) {
		object->model.init(this, modelPath);
		object->texture.init(this, texturePath);
		std::vector<DescriptorSetElement> descriptorSetElements;
		descriptorSetElements.resize(descriptorSetLayoutObject.set.size());
		for (int i = 0; i < descriptorSetLayoutObject.set.size(); i++) {
			descriptorSetElements[i].binding = descriptorSetLayoutObject.set[i];
			descriptorSetElements[i].type = descriptorSetLayoutObject.descriptorSetElementType[i];
			descriptorSetElements[i].size = getSizeForObject(descriptorSetLayoutObject.descriptorSetElementType[i], true);
			descriptorSetElements[i].tex = getPointerOfTexture(descriptorSetLayoutObject.descriptorSetElementType[i], object);
		}
		object->descriptorSet.init(this, &descriptorSetLayout, { descriptorSetElements });
	}

	Texture* getPointerOfTextureForMultipleObject(DescriptorSetElementType descriptorSetElementType, MultipleObject* object) {
		if (descriptorSetElementType == TEXTURE) {
			return &object->texture;
		}
		else {
			return nullptr;
		}
	}

	void multipleInstanceObjectInit(MultipleObject* object, std::string modelPath, std::string texturePath, DescriptorSetLayout descriptorSetLayout, int numberOfInstances, DescriptorSetLayoutObject descriptorSetLayoutObject) {
		object->model.init(this, modelPath);
		object->texture.init(this, texturePath);
		object->descriptorSets.resize(numberOfInstances);
		std::vector<DescriptorSetElement> descriptorSetElements;
		descriptorSetElements.resize(descriptorSetLayoutObject.set.size());
		for (int i = 0; i < descriptorSetLayoutObject.set.size(); i++) {
			descriptorSetElements[i].binding = descriptorSetLayoutObject.set[i];
			descriptorSetElements[i].type = descriptorSetLayoutObject.descriptorSetElementType[i];
			descriptorSetElements[i].size = getSizeForObject(descriptorSetLayoutObject.descriptorSetElementType[i], true);
			descriptorSetElements[i].tex = getPointerOfTextureForMultipleObject(descriptorSetLayoutObject.descriptorSetElementType[i], object);
		}
		for (int i = 0; i < numberOfInstances; i++) {
			object->descriptorSets[i].init(this, &descriptorSetLayout, { descriptorSetElements });
		}
	}

	void descriptorSetInit(DescriptorSet* descriptorSet, DescriptorSetLayout descriptorSetLayout, DescriptorSetLayoutObject descriptorSetLayoutObject) {
		std::vector<DescriptorSetElement> descriptorSetElements;
		descriptorSetElements.resize(descriptorSetLayoutObject.set.size());
		for (int i = 0; i < descriptorSetLayoutObject.set.size(); i++) {
			descriptorSetElements[i].binding = descriptorSetLayoutObject.set[i];
			descriptorSetElements[i].type = descriptorSetLayoutObject.descriptorSetElementType[i];
			descriptorSetElements[i].size = getSizeForObject(descriptorSetLayoutObject.descriptorSetElementType[i], false);
			descriptorSetElements[i].tex = nullptr;
		}
		descriptorSet->init(this, &descriptorSetLayout, { descriptorSetElements });
	}

	//Draw the global descriptor set.
	void drawSingleInstanceInGlobal(VkCommandBuffer commandBuffer, int currentImage,
		Pipeline pipeline, DescriptorSet descriptorSet, int firstDescriptorSetUsed, int numberOfDescriptor) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.pipelineLayout, firstDescriptorSetUsed, numberOfDescriptor, &descriptorSet.descriptorSets[currentImage],
			0, nullptr);
	}

	//Draw single instance of object.
	void drawSingleInstance(VkCommandBuffer commandBuffer, int currentImage,
		Pipeline pipeline, Object object, int firstDescriptorSetUsed, int numberOfDescriptor) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.graphicsPipeline);

		VkBuffer vertexBuffers[] = { object.model.vertexBuffer };
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, object.model.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.pipelineLayout, firstDescriptorSetUsed, numberOfDescriptor, &object.descriptorSet.descriptorSets[currentImage],
			0, nullptr);

		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(object.model.indices.size()), 1, 0, 0, 0);
	}

	//Draw multiple instances of an object.
	void drawMultipleInstance(VkCommandBuffer commandBuffer, int currentImage,
		Pipeline pipeline, MultipleObject object, int firstDescriptorSetUsed, int numberOfDescriptor) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.graphicsPipeline);

		VkBuffer vertexBuffers[] = { object.model.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, object.model.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < object.descriptorSets.size(); i++) {
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline.pipelineLayout, firstDescriptorSetUsed, numberOfDescriptor, &object.descriptorSets[i].descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(object.model.indices.size()), 1, 0, 0, 0);
		}
	}

	//Update an object.
	void updateObject(Object object, UniformBufferObject ubo, uint32_t currentImage) {
		void* data;
		vkMapMemory(device, object.descriptorSet.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, object.descriptorSet.uniformBuffersMemory[0][currentImage]);
	}

	//Update a particular instance of an object.
	void updateOneInstanceOfObject(MultipleObject object, UniformBufferObject ubo, uint32_t currentImage, int descriptorSetInstance) {
		void* data;
		vkMapMemory(device, object.descriptorSets[descriptorSetInstance].uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, object.descriptorSets[descriptorSetInstance].uniformBuffersMemory[0][currentImage]);
	}

	void loadFileContents() {
		std::ifstream in("controller/gamecontrollerdb.txt");
		std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

		const char* mappings = contents.c_str();
		glfwUpdateGamepadMappings(mappings);
	}
};

// This is the main: probably you do not need to touch this!
int main() {
	MyProject app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}