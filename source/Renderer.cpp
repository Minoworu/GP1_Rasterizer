//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(static_cast<float>((m_Width / m_Height)), 60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);
	Render_W1_Part1();
	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
	auto vertices_world = vertices_in;
	for (auto& v : vertices_world)
	{
		m_Camera.viewMatrix.TransformPoint(v.position);
		v.position.x /= (m_Camera.fov * m_Camera.aspectRatio);
		v.position.y /= m_Camera.fov;
		v.position.x /= v.position.z;
		v.position.y /= v.position.z;
	}
	vertices_out = vertices_world;
}

void dae::Renderer::Render_W1_Part1() // Rasterizer
{

	std::vector<Vertex> vertices_world{
		{{0.f,2.f,0.f}},
		{{1.f,0.f,0.f}},
		{{-1.f,0.f,0.f} },
	};
	std::vector<Vertex> vertices_ndc;
	VertexTransformationFunction(vertices_world, vertices_ndc);
	std::vector<Vector2> vertices_SS;
	for (auto& v : vertices_ndc)
	{
		vertices_SS.push_back({ static_cast<float>((v.position.x + 1) / 2 * m_Width),static_cast<float>((1 - v.position.y) / 2 * m_Height) });
	}

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel{ static_cast<float>(px),static_cast<float>(py) };
			Vector2 p0ToPixel = pixel - vertices_SS[0];
			Vector2 edge1 = { vertices_SS[1] - vertices_SS[0] };
			if (Vector2::Cross(edge1, p0ToPixel) < 0)
			{
				continue; // go to next pixel
			}
			Vector2 p1ToPixel = pixel - vertices_SS[1];
			Vector2 edge2 = { vertices_SS[2] - vertices_SS[1] };
			if (Vector2::Cross(edge2, p1ToPixel) < 0)
			{
				continue;
			}
			Vector2 p2ToPixel = pixel - vertices_SS[2];
			Vector2 edge3 = { vertices_SS[0] - vertices_SS[2] };
			if (Vector2::Cross(edge3, p2ToPixel) < 0)
			{
				continue;
			}
			// if pixel is in triangle
			ColorRGB finalColor{ 1,1,1 };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}

		//float gradient = px / static_cast<float>(m_Width);
		//gradient += py / static_cast<float>(m_Width);
		//gradient /= 2.0f;


	}

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
