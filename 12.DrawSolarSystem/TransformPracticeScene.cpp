#include "pch.h"
#include "InputManager.h"
#include "D2DTransform.h"
#include "SolarSystemRenderer.h"
#include "TransformPracticeScene.h"

using TestRenderer = myspace::D2DRenderer;
using Vec2 = MYHelper::Vector2F;

class SolarObject
{
	SolarObject() = delete;
	SolarObject(const SolarObject&) = delete;
	void operator=(const SolarObject&) = delete;

public:
	SolarObject(ComPtr<ID2D1Bitmap1>& bitmap)
	{
		m_BitmapPtr = bitmap;

		++s_id;
		m_name += std::to_wstring(s_id); // ID�� �̸��� �߰�

		m_renderTM = MYTM::MakeRenderMatrix(true);

		D2D1_SIZE_F size = { m_rect.right - m_rect.left, m_rect.bottom - m_rect.top };

		//m_transform.SetPivotPreset(D2DTM::PivotPreset::TopLeft, size);
		//m_transform.SetPivotPreset(D2DTM::PivotPreset::BottomRight, size);
		m_transform.SetPivotPreset(D2DTM::PivotPreset::Center, size);
		m_orbit_transform.SetPivotPreset(D2DTM::PivotPreset::Center, size);
	}

	~SolarObject() { --s_id; }

	void Update(float deltaTime)
	{
		if (m_isSelfRotation)
		{
			m_transform.Rotate(deltaTime * 36.f); // �ڱ� ȸ��
		}
	}

	void Draw(TestRenderer& testRender, D2D1::Matrix3x2F viewTM)
	{
		static  D2D1_RECT_F s_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f);

		D2D1::Matrix3x2F worldTM = m_transform.GetWorldMatrix();

		D2D1::Matrix3x2F finalTM = m_renderTM * worldTM * viewTM;

		D2D1::ColorF boxColor = D2D1::ColorF::LightGray;

		testRender.SetTransform(finalTM);
		testRender.DrawRectangle(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom, boxColor);

		D2D1_RECT_F dest = D2D1::RectF(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom);

		testRender.DrawBitmap(m_BitmapPtr.Get(), dest);
		testRender.DrawMessage(m_name.c_str(), s_rect.left, s_rect.top, 200, 50, D2D1::ColorF::Black);
	}

	static int GetID() { return s_id; }

	void SetPosition(const Vec2& pos)
	{
		m_transform.SetPosition(pos);
		m_orbit_transform.SetPosition(pos);
	}

	void Move(const Vec2& offset)
	{
		m_transform.Translate(offset);
		m_orbit_transform.Translate(offset);
	}

	void SelfRotate(float angle)
	{
		m_transform.Rotate(angle);
	}

	void OrbitRotate(float angle)
	{
		m_orbit_transform.Rotate(angle);
	}

	void ToggleSelected()
	{
		m_isSelected = !m_isSelected;
	}

	bool IsSelected() const
	{
		return m_isSelected;
	}

	void OnSelfRotation()
	{
		m_isSelfRotation = true;
	}

	void OffSelfRotation()
	{
		m_isSelfRotation = false;
	}

	D2DTM::Transform* GetTransform()
	{
		return &m_transform;
	}

	D2DTM::Transform* GetOrbitTransform()
	{
		return &m_orbit_transform;
	}

	void SetParent(SolarObject* parent)
	{
		assert(parent != nullptr);

		if (nullptr != m_transform.GetParent() || nullptr != m_orbit_transform.GetParent())
		{
			// �̹� �θ� �ִٸ� �θ� ���踦 �����մϴ�.
			m_transform.DetachFromParent();
			m_orbit_transform.DetachFromParent();
		}

		m_transform.SetParent(parent->GetOrbitTransform());
		m_orbit_transform.SetParent(parent->GetOrbitTransform());
	}

	void DetachFromParent()
	{
		m_transform.DetachFromParent();
	}

	void SetLeader(bool isLeader)
	{
		m_isLeader = isLeader;
	}

private:
	D2DTM::Transform m_transform;
	D2DTM::Transform m_orbit_transform;

	MAT3X2F m_renderTM; // ������ ��ȯ ���

	D2D1_RECT_F m_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f);

	std::wstring m_name = L"";

	bool m_isSelected = false;
	bool m_isLeader = false; // ���� �ڽ� ����

	bool m_isSelfRotation = false; // �ڱ� ȸ�� ����

	ComPtr<ID2D1Bitmap1> m_BitmapPtr;

	static int s_id; // static ��� ������ ID�� �����մϴ�. (������)

};

int SolarObject::s_id = 0;

TransformPracticeScene::~TransformPracticeScene()
{
	for (auto& solar : m_SolarObjects)
	{
		delete solar;
	}
}

void TransformPracticeScene::SetUp(HWND hWnd)
{
	constexpr int defaultGameObjectCount = 100;

	m_SolarObjects.reserve(defaultGameObjectCount);

    m_hWnd = hWnd;

    SetWindowText(m_hWnd, 
    L"F2 : ���� On, F3 : ���� Off, �����̽��� : ���� On/Off");

    std::cout << "�¾��� ������ �ؾ� �մϴ�." << std::endl;
    std::cout << "�༺���� ������ �ϸ� ���ÿ� �¿��� ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�."<< std::endl;
    std::cout << "���� ������ �ϸ鼭 ���ÿ� ������ ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�." << std::endl;
    std::cout << "ȸ�� �ӵ��� �����Ӱ� �����ϼ���." << std::endl;

	SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/s.png", *m_SunBitmapPtr.GetAddressOf());
	SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/e.png", *m_EarthBitmapPtr.GetAddressOf());
	SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/m.png", *m_MoonBitmapPtr.GetAddressOf());
	SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/Saturn.png", *m_SaturnBitmapPtr.GetAddressOf());

   
    RECT rc;
    if (::GetClientRect(hWnd, &rc))
    {
        float w = static_cast<float>(rc.right - rc.left);
        float h = static_cast<float>(rc.bottom - rc.top);

        m_UnityCamera.SetScreenSize(w, h);
    }

	SetSolar();
}

void TransformPracticeScene::Tick(float deltaTime)
{
	ProcessKeyboardEvents();

	for (auto& solar : m_SolarObjects)
	{
		solar->Update(deltaTime);
		if (m_IsOrbitRotate)
			solar->OrbitRotate(0.3f);
	}

	// ī�޶� ������Ʈ

	MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();

	MAT3X2F renderTM = MYTM::MakeRenderMatrix(true); // ī�޶� ��ġ ������ ��Ʈ����

	MAT3X2F finalTM = renderTM * cameraTM;

	// ������

	static myspace::D2DRenderer& globalRenderer = SolarSystemRenderer::Instance();

	wchar_t buffer[128] = L"";
	MYTM::MakeMatrixToString(cameraTM, buffer, 128);

	globalRenderer.RenderBegin();

	globalRenderer.SetTransform(finalTM);

	for (auto& solar : m_SolarObjects)
	{
		solar->Draw(globalRenderer, cameraTM);
	}

	globalRenderer.RenderEnd();
}

void TransformPracticeScene::OnResize(int width, int height)
{ 
	// ������ ũ�� ���� �� ī�޶��� ȭ�� ũ�⸦ ������Ʈ
	m_UnityCamera.SetScreenSize(width, height);
}

void TransformPracticeScene::ProcessKeyboardEvents()
{
	// �¾�� ������� ����
	if (InputManager::Instance().GetKeyPressed(VK_F1))
	{
		
	}

	if (InputManager::Instance().GetKeyPressed(VK_F2))
	{
		SetSolarSelfRotation();
	}

	if (InputManager::Instance().GetKeyPressed(VK_F3))
	{
		OffSolarSelfRotation();
	}

	// �¾� ȸ��
	if (InputManager::Instance().GetKeyPressed(VK_SPACE) && !m_SolarObjects.empty())
	{
		SetOrbitRotation();
	}
}

void TransformPracticeScene::SetOrbitRotation()
{
	m_IsOrbitRotate = !m_IsOrbitRotate;
} 

void TransformPracticeScene::OrbitSolar()
{
	for (auto& solar : m_SolarObjects)
	{
		solar->OrbitRotate(0.3f);
	}
}

void TransformPracticeScene::SetSolar()
{
	AddSun();
	AddEarth();
	AddMoon();
	AddSaturn();
}

void TransformPracticeScene::AddSun()
{
	if (!m_SolarObjects.empty())
		return;

	D2D1_POINT_2F worldPt = { 0, 0 };

	std::cout << "x : " << worldPt.x << " y : " << worldPt.y << std::endl;

	SolarObject* pNewBox = new SolarObject(m_SunBitmapPtr);

	pNewBox->SetPosition(Vec2(worldPt.x, worldPt.y));

	pNewBox->GetTransform()->SetScale({ 1.5, 1.5 });

	m_SolarObjects.push_back(pNewBox);
	m_PlanetObjects.push_back(pNewBox);
}

void TransformPracticeScene::AddEarth()
{
	D2D1_POINT_2F worldPt = { 150, 150 };

	std::cout << "x : " << worldPt.x << " y : " << worldPt.y << std::endl;

	SolarObject* pNewBox = new SolarObject(m_EarthBitmapPtr);

	pNewBox->SetPosition(Vec2(worldPt.x, worldPt.y));

	pNewBox->GetTransform()->SetScale({ 0.5, 0.5 });

	pNewBox->SetParent(m_PlanetObjects.front());

	m_SolarObjects.push_back(pNewBox);
	m_PlanetObjects.push_back(pNewBox);
}

void TransformPracticeScene::AddMoon()
{
	D2D1_POINT_2F worldPt = { 190.f, 190.f };

	std::cout << "x : " << worldPt.x << " y : " << worldPt.y << std::endl;

	SolarObject* pNewBox = new SolarObject(m_MoonBitmapPtr);

	pNewBox->SetPosition(Vec2(worldPt.x, worldPt.y));

	pNewBox->GetTransform()->SetScale({ 0.2, 0.2 });

	pNewBox->SetParent(m_PlanetObjects.back());

	m_SolarObjects.push_back(pNewBox);
	m_PlanetObjects.push_back(pNewBox);
}

void TransformPracticeScene::AddSaturn()
{
	D2D1_POINT_2F worldPt = { 250.f, 250.f };

	SolarObject* pNewBox = new SolarObject(m_SaturnBitmapPtr);

	pNewBox->SetPosition(Vec2(worldPt.x, worldPt.y));

	pNewBox->GetTransform()->SetScale({ 0.5, 0.5 });

	pNewBox->SetParent(m_PlanetObjects.front());

	m_SolarObjects.push_back(pNewBox);
	m_PlanetObjects.push_back(pNewBox);
}

void TransformPracticeScene::SetSolarSelfRotation()
{
	for (auto& planet : m_PlanetObjects)  // �¾��̶� �༺�� ����
	{
		planet->OnSelfRotation();
	}
}

void TransformPracticeScene::OffSolarSelfRotation()
{
	for (auto& planet : m_PlanetObjects)  // �¾��̶� �༺�� ����
	{
		planet->OffSelfRotation();
	}
}

void TransformPracticeScene::ClearSolarObjects()
{
	for (auto& solar : m_SolarObjects)
	{
		delete solar;
	}

	m_SolarObjects.clear();
}

