#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "box.h"
#include "Octree.h"
#include "ray.h"
#include "ParticleSystem.h"
#include "Particle.h"
#include "Particle.h"
#include "ParticleEmitter.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool  doPointSelection();
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
		void subDivideBox8(const Box &b, vector<Box> & boxList);

		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
    
        void loadVbo();

        // Camera Section
        //
		ofEasyCam mainCam;
        ofCamera trackingCam;
        ofCamera downwardCam;
        ofCamera topCam;
        ofCamera attachedCam;
        ofCamera distCam;
        ofCamera *theCam;
    
    
    
		ofxAssimpModelLoader mars, rover;
		ofLight light;
		Box boundingBox;
        int levelDivid = 12;
        ofVec3f center;
        TreeNode foundNode;
        ofxAssimpModelLoader spaceshipModel;
    
        Octree zOTree;
    
        bool bModelLoaded = false;
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
        bool bSelectedBoxPoint;
        bool bPause = true;
		
		bool bRoverLoaded;
		bool bTerrainSelected;
        bool soundLoaded;
	
        ofSoundPlayer rocketThruster;
    
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;


		const float selectionRange = 4.0;
        int prevKey = -9999;
    
        Vector3 spaceShipSize;
        Vector3 spaceShipCenter;
        Particle SpaceShipParticle;
        Particle LandingArea;
        ParticleSystem *forceSys;
        ParticleSystem *landingSys;
    
        // Physics Forces
        //
        GravityForce grav;
        TurbulenceForce tur;
        ImpulseForce movingImpulse;
        ImpulseRadialForce radialForce;
    
        // Exhaust System
        //
        ParticleEmitter exhaust;
    
        // textures
        //
        ofTexture  particleTex;
    
        // shaders
        //
        ofVbo vbo;
        ofShader shader;
    
        // Lighting
        //
        ofLight keyLight, fillLight, rimLight;
};
