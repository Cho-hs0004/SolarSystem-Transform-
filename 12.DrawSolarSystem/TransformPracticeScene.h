#pragma once
#include "OnlyForTestScene.h"
#include "Camera2D.h"
#include <list>
#include <vector>
#include <wrl/client.h>
#include <d2d1_1.h>

class SolarObject;

class TransformPracticeScene : public OnlyForTestScene
{
public:
    TransformPracticeScene() = default;
    virtual ~TransformPracticeScene();
    
    void SetUp(HWND hWnd) override;

    void Tick(float deltaTime) override;

    void OnResize(int width, int height) override;

    

private:

    void ProcessKeyboardEvents();
    void OrbitSolar();

    void SetSolar();

    void AddSun();
    void AddEarth();
    void AddMoon();
    void AddSaturn();

	void SetSolarSelfRotation();
    void OffSolarSelfRotation();

    void SetOrbitRotation();

    void ClearSolarObjects();

	ComPtr<ID2D1Bitmap1> m_SunBitmapPtr;
	ComPtr<ID2D1Bitmap1> m_EarthBitmapPtr;
	ComPtr<ID2D1Bitmap1> m_MoonBitmapPtr;
	ComPtr<ID2D1Bitmap1> m_SaturnBitmapPtr;

	std::vector<SolarObject*> m_SolarObjects;
	std::vector<SolarObject*> m_PlanetObjects;
   
    UnityCamera m_UnityCamera;

    bool m_IsOrbitRotate = false;
};

