#pragma once

#include <iostream>
#include "../VGL-2D/Rectangle.h"
#include "../Platform/Vulkan/VkImage.h"
#include "../Platform/Vulkan/VkVertexArray.h"

namespace vgl
{
	struct Particle : public Transform3D
	{
		Vector4f Color = { 1, 1, 1, 1 };
		Vector3f Velocity = { 0, 0, 0 };
		float Life = 1.0f;
	};

	class Particle_Emitter
	{
		public:
			Particle_Emitter()
			{
				
			}
			~Particle_Emitter()
			{

			}

			// Particle Start Velocity
			Vector3f pVelocity;
			Vector3f Position;
			uint32_t nMaxParticles = 500;
			uint32_t nSpawnParticles = 20;
			Vector4f intColor = { 0, 0, 1, 1 };

			vk::Image Texture;

			void create()
			{
				m_Particles.resize(nMaxParticles);
				m_InstanceData.resize(nMaxParticles);

				vgl::BufferLayout _ParticleInstancesLayout = { {
					{ vgl::ShaderDataType::Float, 2, "inInstanceLife" },
					{ vgl::ShaderDataType::Vec4f, 3, "inInstanceColor" },
					{ vgl::ShaderDataType::Vec4f, 4, "inInstanceOutlineColor" },
					{ vgl::ShaderDataType::Vec2f, 5, "inInstanceOutlineThickness" },
					{ vgl::ShaderDataType::Mat4f, 6, "in_InstanceTransform" },
				},1 };

				m_InstanceDataVBuf.setLayout(_ParticleInstancesLayout);
				m_InstanceDataVBuf.allocateHV(m_InstanceData.size() * sizeof(InstanceData));
			}

			void update(const double& p_DeltaTime)
			{
				for (uint32_t i = 0; i < nSpawnParticles; i++)
					Respawn(GetFreeParticle(), { -30, 0, 30 });

				nIdleParticles += nSpawnParticles;

				for (uint32_t i = 0; i < nMaxParticles; i++) {
					Particle& p = m_Particles[i];
					p.Life -= p_DeltaTime;

					if (p.Life > 0.0f) {
						Vector3f tmp = p.Velocity * (float)p_DeltaTime;
						p.move(tmp);
						p.Color.r -= p_DeltaTime;
						//p.Color.r = Math::lerp(p.Color.r, intColor.r, p_DeltaTime * 2.5f);
						//p.Color.g = Math::lerp(p.Color.g, intColor.g, p_DeltaTime * 2.5f);
						//p.Color.b = Math::lerp(p.Color.b, intColor.b, p_DeltaTime * 2.5f);
						//p.Color.a = Math::lerp(p.Color.a, intColor.a, p_DeltaTime * 2.5f);

						m_InstanceData[i].Model = m_Particles[i].model;
						m_InstanceData[i].Color = m_Particles[i].Color;
						m_InstanceData[i].OutlineColor.xyz = { 0,0,0 };
						m_InstanceData[i].OutlineThickness = { 0, 0 };
						m_InstanceData[i].Life = m_Particles[i].Life;
					} else nIdleParticles--;
				}

				m_InstanceDataVBuf.copy(m_InstanceData.data(), 0, m_InstanceData.size() * sizeof(InstanceData));
			}

		private:
			friend class vk::Renderer;

			struct InstanceData
			{
				alignas(4)  float Life = 0;
				alignas(16) Vector4f Color;
				alignas(16) Vector4f OutlineColor;
				alignas(8)  Vector2f OutlineThickness;
				alignas(16) Matrix4f Model;
			};

			uint32_t nIdleParticles = 0;
			std::vector<InstanceData> m_InstanceData;
			
			vk::VertexBuffer m_InstanceDataVBuf;
			vk::VertexArray m_Vao;

			uint32_t last_idle_particle = 0;
			Particle& GetFreeParticle()
			{
				for (uint32_t i = last_idle_particle; i < nMaxParticles; i++) {
					if (m_Particles[i].Life <= 0.0f) {
						last_idle_particle = i;
						return m_Particles[i];
					}
				}

				for (uint32_t i = 0; i < last_idle_particle; i++) {
					if (m_Particles[i].Life <= 0.0f) {
						last_idle_particle = i;
						return m_Particles[i];
					}
				}

				last_idle_particle = 0;
				return m_Particles[0];
			}
			void Respawn(Particle& p_Particle, Vector3f p_Offset)
			{
				p_Particle.setPosition(Position + p_Offset * Math::getRandomNumberIND(0.0f, 10));
				p_Particle.Life = 1.0f;
				p_Particle.Velocity = pVelocity * 0.1f;
				p_Particle.setScale(10, 10, 10);
				p_Particle.Color = { 1, 1, 1, 1 };
			}

			vk::Descriptor m_Descriptor;
			

			std::vector<Particle> m_Particles;
	};
}