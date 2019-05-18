
//  Student Name:  Jinshan Jiang
//  Date: 05/16/2019


#include "ofApp.h"
#include "Util.h"
#include "Octree.h"
#include <numeric>



//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bRoverLoaded = false;
	bTerrainSelected = true;
    ofSetBackgroundColor(ofColor::black);
//	ofSetWindowShape(1024, 768);
    
    // loading the rocket firing sound
    //
    if(rocketThruster.load("sounds/rocket.wav")) soundLoaded = true;
    
	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);
    
    // Building Octree
    //
    float start, end;
    start = ofGetElapsedTimeMillis();
    zOTree.create(mars.getMesh(0), levelDivid);
    zOTree.randColor(levelDivid);
    
    // Physics
    //
    forceSys = new ParticleSystem();
    grav.setGravity(ofVec3f(0, -0.5, 0));
    tur.setTubulence(ofVec3f(-1,-1,-1), ofVec3f(1,1,1));
    radialForce.setImpulseRadialForce(100);
    movingImpulse.setImpulseForce(1);
    
    forceSys->addForce(&grav);
    forceSys->addForce(&tur);
    forceSys->addForce(&movingImpulse);

    
    
    //Loading model and set measurements
    if (spaceshipModel.loadModel("geo/lander.obj")) {
        spaceshipModel.setScaleNormalization(false);
        bModelLoaded = true;
        SpaceShipParticle.width = spaceshipModel.getSceneMax().x - spaceshipModel.getSceneMin().x;
        SpaceShipParticle.height = spaceshipModel.getSceneMax().y - spaceshipModel.getSceneMin().y;
        SpaceShipParticle.depth = spaceshipModel.getSceneMax().z - spaceshipModel.getSceneMin().z;
        SpaceShipParticle.position = ofVec3f(0, 20, 0);
        spaceshipModel.setPosition(SpaceShipParticle.position.x, SpaceShipParticle.position.y-SpaceShipParticle.height/2, SpaceShipParticle.position.z);
        forceSys->add(SpaceShipParticle);
        
    }
    else cout << "Error: Can't load model:" << " geo/lander.obj" << endl;
    
    // Landing Area
    //
    landingSys = new ParticleSystem();
    LandingArea.position = ofVec3f(0, 0, 0);
    LandingArea.width = 10;
    LandingArea.depth = 10;
    LandingArea.height = 2;
    landingSys->add(LandingArea);
    
    // Camera Section
    //
    mainCam.setDistance(10);
    mainCam.setNearClip(.1);
    mainCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    downwardCam.setNearClip(0.25);
    attachedCam.setNearClip(0.25);
    ofSetVerticalSync(true);
    mainCam.disableMouseInput();
    ofEnableSmoothing();
    ofEnableDepthTest();
    
    theCam = &trackingCam;
    
    
    
    attachedCam.lookAt(glm::vec3(0, -90, 0));
    downwardCam.lookAt(glm::vec3(0, -90, 0));
    
    
    // texture loading
    //
    ofDisableArbTex();     // disable rectangular textures
    
    // load textures
    //
    if (!ofLoadImage(particleTex, "images/dot.png")) {
        cout << "Particle Texture File: images/dot.png not found" << endl;
        ofExit();
    }
    
    // load the shader
    //
#ifdef TARGET_OPENGLES
    shader.load("shaders_gles/shader");
#else
    shader.load("shaders/shader");
#endif
    
    // set up the exhaust
    //
    exhaust.sys->addForce(&tur);
    exhaust.sys->addForce(&grav);
    exhaust.sys->addForce(&radialForce);
    
    exhaust.setVelocity(ofVec3f(0, 0, 0));
    exhaust.setOneShot(true);
    exhaust.setEmitterType(RadialEmitter);
    exhaust.setGroupSize(500);
    
    // set up Lighting
    //
    ofEnableLighting();
    
    // Setup 3 - Light System
    //
    keyLight.setup();
    keyLight.enable();
    keyLight.setAreaLight(1, 1);
    keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    keyLight.setSpecularColor(ofFloatColor(1, 1, 0.2));
    
    keyLight.rotate(45, ofVec3f(0, 1, 0));
    keyLight.rotate(-45, ofVec3f(1, 0, 0));
    keyLight.setPosition(5, 5, 5);
    
    fillLight.setup();
    fillLight.enable();
    fillLight.setSpotlight();
    fillLight.setScale(.05);
    fillLight.setSpotlightCutOff(15);
    fillLight.setAttenuation(2, .001, .001);
    fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    fillLight.setSpecularColor(ofFloatColor(0, 0.9, 0.3));
    fillLight.rotate(-10, ofVec3f(1, 0, 0));
    fillLight.rotate(-45, ofVec3f(0, 1, 0));
    fillLight.setPosition(-5, 5, 5);
    
    rimLight.setup();
    rimLight.enable();
    rimLight.setSpotlight();
    rimLight.setScale(.05);
    rimLight.setSpotlightCutOff(30);
    rimLight.setAttenuation(.2, .001, .001);
    rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    rimLight.setSpecularColor(ofFloatColor(0.9, 0.2, 0.1));
    rimLight.rotate(180, ofVec3f(0, 1, 0));
    rimLight.setPosition(0, 5, -7);

}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
    if (exhaust.sys->particles.size() < 1) return;
    
    vector<ofVec3f> sizes;
    vector<ofVec3f> points;
    for (int i = 0; i < exhaust.sys->particles.size(); i++) {
        points.push_back(exhaust.sys->particles[i].position);
        sizes.push_back(ofVec3f(5));
    }
    // upload the data to the vbo
    //
    int total = (int)points.size();
    vbo.clear();
    vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
    vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    if(!bPause){
        if(forceSys->onTallGround(landingSys)){
            forceSys->setVelocity(ofVec3f(0,0,0));
            grav.setGravity(ofVec3f(0,0,0));
        }
        
        forceSys->update();
        landingSys->update();
        spaceshipModel.update();
        exhaust.update();
    }
}
//--------------------------------------------------------------
void ofApp::draw(){
    loadVbo();
    
    theCam->begin();
	//mainCam.draw();
    //trackingCam.draw();
	ofPushMatrix();
    //keyLight.draw();
    //fillLight.draw();
    //rimLight.draw();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bRoverLoaded) {
			rover.drawWireframe();
			//if (!bTerrainSelected) //drawAxis(rover.getPosition());
		}
		//if (bTerrainSelected) //drawAxis(ofVec3f(0, 0, 0));
    }
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();

		if (bRoverLoaded) {
			rover.drawFaces();
			//if (!bTerrainSelected) //drawAxis(rover.getPosition());
		}
		//if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		//ofDrawSphere(selectedPoint, .1);
	}

	ofNoFill();

    ofMultMatrix(mars.getModelMatrix());
    zOTree.draw(zOTree.root, levelDivid, 0);
    if(bSelectedBoxPoint){
        ofSetColor(ofColor::green);
        //ofDrawSphere(center, 1);
    }

    // draw the model (model has textures on it)
    //
    
    if (bModelLoaded) {
        spaceshipModel.setPosition(forceSys->particles[0].position.x, forceSys->particles[0].position.y-SpaceShipParticle.height/2, forceSys->particles[0].position.z);
        if (!bWireframe) spaceshipModel.drawFaces();
        else spaceshipModel.drawWireframe();
        ofSetColor(ofColor::orangeRed);
        forceSys->draw();
        
        landingSys->draw();
        ofFill();
        ofDrawBox(LandingArea.position, LandingArea.width, LandingArea.height, LandingArea.depth);

    }
    ofNoFill();
    // Tracking Camera
    trackingCam.setPosition(glm::vec3(spaceshipModel.getPosition().x, spaceshipModel.getPosition().y-4, spaceshipModel.getPosition().z+20));
    trackingCam.lookAt(glm::vec3(forceSys->particles[0].position.x, forceSys->particles[0].position.y, forceSys->particles[0].position.z));
    
    // Bottom Camera
    downwardCam.setPosition(glm::vec3(spaceshipModel.getPosition().x/2, spaceshipModel.getPosition().y-SpaceShipParticle.height/2+3, spaceshipModel.getPosition().z/2));
    //downwardCam.draw();
    
    
    // Top Camera
    topCam.setPosition(glm::vec3(spaceshipModel.getPosition().x/2, spaceshipModel.getPosition().y+20, spaceshipModel.getPosition().z/2));
    topCam.lookAt(glm::vec3(forceSys->particles[0].position.x, forceSys->particles[0].position.y, forceSys->particles[0].position.z));
    
    // Attached Cam needs more work
    //
    attachedCam.setPosition(glm::vec3(forceSys->particles[0].position.x, forceSys->particles[0].position.y+3.6, forceSys->particles[0].position.z-1.7));
    //attachedCam.draw();
    
    // Distance Camera
    //
    distCam.setPosition(glm::vec3(4.89124, 10.14334, 118.237));
    distCam.lookAt(glm::vec3(forceSys->particles[0].position.x, forceSys->particles[0].position.y-5, forceSys->particles[0].position.z));
    
	ofPopMatrix();
    theCam->end();
    
    // Shader Section
    //
    glDepthMask(GL_FALSE);
    
    ofSetColor(0, 0, 0);
    
    // this makes everything look glowy :)
    //
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnablePointSprites();
    
    shader.begin();
    theCam->begin();
    exhaust.setPosition(ofVec3f(forceSys->particles[0].position.x, forceSys->particles[0].position.y-2, forceSys->particles[0].position.z));
    exhaust.draw();
    particleTex.bind();
    vbo.draw(GL_POINTS, 0, (int)exhaust.sys->particles.size());
    particleTex.unbind();
    
    
    theCam->end();
    shader.end();
    
    ofDisablePointSprites();
    ofDisableBlendMode();
    ofEnableAlphaBlending();
    
    // set back the depth mask
    //
    glDepthMask(GL_TRUE);
    
    // Spaceship Bottom Ray
    ofVec3f rayPoint = downwardCam.screenToWorld(ofVec3f(spaceshipModel.getPosition().x/2, spaceshipModel.getPosition().y, spaceshipModel.getPosition().z/2));
    ofVec3f rayDir = rayPoint - downwardCam.getPosition();
    rayDir.normalize();
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
                  Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    if (zOTree.intersect(ray, zOTree.root, foundNode)) {
        center = ofVec3f(foundNode.box.center().x(),foundNode.box.center().y(),foundNode.box.center().z());
        cout << foundNode.box.center().x() << " " << foundNode.box.center().y()  << " "<< foundNode.box.center().z() << endl;
        //bSelectedBoxPoint = true;
    }
}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
    if (key == prevKey) {
        return;
    }
    switch (key) {
        case ' ':
            if(soundLoaded) rocketThruster.play();
            exhaust.sys->reset();
            exhaust.start();
            //forceSys->setVelocity(ofVec3f(0,0.5,0));
            movingImpulse.setDir(ofVec3f(0,5,0));
            key = prevKey;
            break;
        case 'd':
            forceSys->setVelocity(ofVec3f(0,-5,0));
            key = prevKey;
            break;
        case OF_KEY_LEFT:
            if(soundLoaded) rocketThruster.play();
            //forceSys->setVelocity(ofVec3f(-0.3,0,0));
            exhaust.sys->reset();
            exhaust.start();
            movingImpulse.setDir(ofVec3f(-1,1,0));
            key = prevKey;
            break;
        case OF_KEY_RIGHT:
            if(soundLoaded) rocketThruster.play();
            //forceSys->setVelocity(ofVec3f(0.3,0,0));
            exhaust.sys->reset();
            exhaust.start();
            movingImpulse.setDir(ofVec3f(1,1,0));
            key = prevKey;
            break;
        case OF_KEY_UP:
            if(soundLoaded) rocketThruster.play();
            //forceSys->setVelocity(ofVec3f(0,0,0.3));
            exhaust.sys->reset();
            exhaust.start();
            movingImpulse.setDir(ofVec3f(0,1,1));
            key = prevKey;
            break;
        case OF_KEY_DOWN:
            if(soundLoaded) rocketThruster.play();
            //forceSys->setVelocity(ofVec3f(0,0,-0.3));
            exhaust.sys->reset();
            exhaust.start();
            movingImpulse.setDir(ofVec3f(0,1,-1));
            key = prevKey;
            break;
            
            
        case OF_KEY_RETURN:
            bPause = !bPause;
            break;
        case OF_KEY_F1:
            theCam = &mainCam;
            break;
        case OF_KEY_F2:
            theCam = &trackingCam;
            break;
        case OF_KEY_F3:
            theCam = &downwardCam;
            break;
        case OF_KEY_F4:
            theCam = &topCam;
            break;
        case OF_KEY_F5:
            theCam = &attachedCam;
            break;
        case OF_KEY_F6:
            theCam = &distCam;
            break;
        case 'C':
        case 'c':
            if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
            else mainCam.enableMouseInput();
            break;
        case 'F':
        case 'f':
            ofToggleFullscreen();
            break;
        case 'H':
        case 'h':
            break;
        case 'r':
            mainCam.reset();
            break;
        case 's':
            savePicture();
            break;
        case 't':
            setCameraTarget();
            break;
        case 'u':
            break;
        case 'v':
            togglePointsDisplay();
            break;
        case 'V':
            break;
        case 'w':
            toggleWireframeMode();
            break;
        case OF_KEY_ALT:
            mainCam.enableMouseInput();
            bAltKeyDown = true;
            break;
        case OF_KEY_CONTROL:
            bCtrlKeyDown = true;
            break;
        case OF_KEY_SHIFT:
            break;
        case OF_KEY_DEL:
            break;
        default:
            break;
    }
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
    switch (key) {
        case OF_KEY_LEFT:
            movingImpulse.setDir(ofVec3f(0,0,0));
            forceSys->setVelocity(ofVec3f(0,0,0));
            break;
        case OF_KEY_RIGHT:
            movingImpulse.setDir(ofVec3f(0,0,0));
            forceSys->setVelocity(ofVec3f(0,0,0));
            break;
        case OF_KEY_UP:
            movingImpulse.setDir(ofVec3f(0,0,0));
            forceSys->setVelocity(ofVec3f(0,0,0));
            break;
        case OF_KEY_DOWN:
            movingImpulse.setDir(ofVec3f(0,0,0));
            forceSys->setVelocity(ofVec3f(0,0,0));
            break;
        case ' ':
            movingImpulse.setDir(ofVec3f(0,0,0));
            forceSys->setVelocity(ofVec3f(0,0,0));
            break;
            
            
            
            
        case OF_KEY_ALT:
            mainCam.disableMouseInput();
            bAltKeyDown = false;
            break;
        case OF_KEY_CONTROL:
            bCtrlKeyDown = false;
            break;
        case OF_KEY_SHIFT:
            break;
        default:
            break;
    }
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = mainCam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));
	//if (level3[1].intersect(ray, -1000, 1000)) cout << "intersects" << endl;

    float start, end;
    start = ofGetElapsedTimeMillis();
    if (zOTree.intersect(ray, zOTree.root, foundNode)) {
        center = ofVec3f(foundNode.box.center().x(),foundNode.box.center().y(),foundNode.box.center().z());
        cout << foundNode.box.center().x() << " " << foundNode.box.center().y()  << " "<< foundNode.box.center().z() << endl;
        bSelectedBoxPoint = true;
    }
    end = ofGetElapsedTimeMillis();
    cout << "Finding intersect used: " << (end-start)/1000 << " s" << endl;
}

//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void ofApp::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	//boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}


//
//  ScreenSpace Selection Method: 
//  This is not the octree method, but will give you an idea of comparison
//  of speed between octree and screenspace.
//
//  Select Target Point on Terrain by comparing distance of mouse to 
//  vertice points projected onto screenspace.
//  if a point is selected, return true, else return false;
//
bool ofApp::doPointSelection() {

	ofMesh mesh = mars.getMesh(0);
	int n = mesh.getNumVertices();
	float nearestDistance = 0;
	int nearestIndex = 0;

	bPointSelected = false;

	ofVec2f mouse(mouseX, mouseY);
	vector<ofVec3f> selection;

	// We check through the mesh vertices to see which ones
	// are "close" to the mouse point in screen space.  If we find 
	// points that are close, we store them in a vector (dynamic array)
	//
	for (int i = 0; i < n; i++) {
		ofVec3f vert = mesh.getVertex(i);
		ofVec3f posScreen = mainCam.worldToScreen(vert);
		float distance = posScreen.distance(mouse);
		if (distance < selectionRange) {
			selection.push_back(vert);
			bPointSelected = true;
		}
	}

	//  if we found selected points, we need to determine which
	//  one is closest to the eye (camera). That one is our selected target.
	//
	if (bPointSelected) {
		float distance = 0;
		for (int i = 0; i < selection.size(); i++) {
			ofVec3f point =  mainCam.worldToCamera(selection[i]);

			// In camera space, the camera is at (0,0,0), so distance from 
			// the camera is simply the length of the point vector
			//
			float curDist = point.length(); 

			if (i == 0 || curDist < distance) {
				distance = curDist;
				selectedPoint = selection[i];
			}
		}
	}
	return bPointSelected;
}

// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), mainCam.getZAxis(), point);

	if (rover.loadModel(dragInfo.files[0])) {
		rover.setScaleNormalization(false);
		rover.setScale(.005, .005, .005);
		rover.setPosition(point.x, point.y, point.z);
		bRoverLoaded = true;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	glm::vec3 mouse(mouseX, mouseY, 0);
	ofVec3f rayPoint = mainCam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}
