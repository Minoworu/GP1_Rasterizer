#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <algorithm>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{};
		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		const float speed{ 10.f };
		const float rotationSpeed{ 5.f * TO_RADIANS };
		float totalPitch{};
		float totalYaw{};
		const float minYaw{ -1.f };
		const float maxYaw{ 1.f };
		
		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _aspectRatio, float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			origin = _origin;
			aspectRatio = _aspectRatio;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			//invView
			invViewMatrix = Matrix{ right,up,forward,origin };
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			viewMatrix = invViewMatrix.Inverse();


			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			const uint8_t* pSprintState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// FORWARD / BACKWARD INPUT 
			origin += forward * speed * (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP]) * deltaTime;
			origin += forward * -speed * (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN]) * deltaTime;
			if (mouseState == SDL_BUTTON_LMASK)
			{
				// using delta time is a bit clunky for mouse movement.
				origin += forward * speed * float(mouseX) * 0.05f;
				origin += forward * -speed * float(mouseY) * 0.05f;
			}
			// SIDEWAY INPUT
			origin += right * speed * (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT]) * deltaTime;
			origin += right * -speed * (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT]) * deltaTime;

			// VERTICAL INPUT
			if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			{
				origin += up * speed * float(mouseX) * 0.05f;
				origin += up * -speed * float(mouseY) * 0.05f;
			}
			// ROTATION
			//https://stackoverflow.com/questions/71030102/how-to-detect-if-left-mousebutton-is-being-held-down-with-sdl2
			if (mouseState == SDL_BUTTON_RMASK)
			{
				totalPitch += -mouseY * (rotationSpeed) * 0.05f;
				totalYaw += mouseX * (rotationSpeed) * 0.05f;
				totalPitch = std::clamp(totalPitch, minYaw, maxYaw);
			}
			forward = Matrix::CreateRotation(totalPitch, totalYaw, 0.f).TransformVector(Vector3::UnitZ);
			/*forward.Normalize();*/
			//Camera Update Logic
			//...

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
