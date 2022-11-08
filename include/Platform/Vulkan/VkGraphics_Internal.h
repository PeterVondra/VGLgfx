#pragma once

#include "VkImage.h"
#include "VkBuffers.h"
#include "VkRenderPass.h"
#include "../../Math/Transform.h"
#include "VkFramebufferAttachment.h"
#include "../../GDefinitions.h"
#include <vector>

namespace vgl
{
	namespace vk
	{
		struct RenderInfo
		{
			bool p_AlphaBlending = false;
			CullMode p_CullMode = CullMode::BackBit;
			PolygonMode p_PolygonMode = PolygonMode::Fill;
			IATopoogy p_IATopology = IATopoogy::TriList;
		};

		class GraphicsContext
		{
			public:

				static void init();

				static void createSkyboxPipelines(RenderPass& p_RenderPass);
				// Pass GBuffer RenderPass
				static void createShapesPipelines(RenderPass& p_RenderPass);

				inline static Shader& getDShadowMapShader() { static Shader m_DShadowMapShader; return m_DShadowMapShader; }
				inline static Shader& getDShadowMapAlbedoShader() { static Shader m_DShadowMapAlbedoShader; return m_DShadowMapAlbedoShader; }

				static ImageCube	generateAtmosphericScatteringCubeMap(Vector2i p_Size, AtmosphericScatteringInfo& p_Info);
				static ImageCube	getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size);
				static ImageCube	generateIrradianceMap(ImageCube& p_CubeMapImage, Vector2i p_Size);
				static ImageCube	generatePreFilteredMaps(ImageCube& p_CubeMapImage, Vector2i p_InitialSize);
				//static Image	generateBRDFLut(Vector2f p_Size);

				static const VertexArray& getRecVao();
				static VertexBuffer& getRecVertexBuffer();
				static IndexBuffer& getRecIndexBuffer();

			private:
				friend class Renderer;

				static g_Pipeline m_TextBoxPipeline;
				static g_Pipeline m_Shape2DPipeline;
				static g_Pipeline m_Circle2DPipeline;
				static g_Pipeline m_ParticlePipeline;
				
				static Shader m_TextBoxShader;
				static Shader m_Shape2DShader;
				static Shader m_Circle2DShader;
				static Shader m_ParticleShader;
				
				inline static g_Pipeline& getAtmosphericScatteringPipeline() { static g_Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }
				inline static g_Pipeline& getSkyBoxPipeline() { static g_Pipeline m_SkyBoxPipeline; return m_SkyBoxPipeline; }
				inline static Shader& getSkyBoxShader() { static Shader m_SkyBoxShader; return m_SkyBoxShader; }
				//static g_Pipeline& getAtmosphericScatteringPipelineC16SF() { static g_Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }
				//static g_Pipeline& getAtmosphericScatteringPipelineC16SFD() { static g_Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }


				inline static Shader& getAtmosphericScatteringShader() { static Shader m_AtmosphericScatteringShader; return m_AtmosphericScatteringShader; }
				//inline static g_Pipeline& getSkyBoxPipeline() { static g_Pipeline m_SkyBoxPipeline; return m_SkyBoxPipeline; }
				//inline static Shader& getSkyBoxShader() { static Shader m_SkyBoxShader; return m_SkyBoxShader; }

				inline static g_Pipeline& getDShadowMapPipeline() { static g_Pipeline m_DShadowMapPipeline; return m_DShadowMapPipeline; }
				inline static g_Pipeline& getDShadowMapAlbedoPipeline() { static g_Pipeline m_DShadowMapAlbedoPipeline; return m_DShadowMapAlbedoPipeline; }
				inline static RenderPass& getDShadowMapRenderPass() { static RenderPass m_DShadowMapRenderPass(RenderPassType::Graphics); return m_DShadowMapRenderPass; }
				inline static VkDescriptorSetLayout& getDShadowMapDescriptorSetLayout() {
					static VkDescriptorSetLayout m_DShadowMapDescriptorSetLayout = 
						ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createDescriptorSetLayout({ {0} });
					return m_DShadowMapDescriptorSetLayout;
				}
				inline static VkDescriptorSetLayout& getDShadowMapAlbedoDescriptorSetLayout() {
					static VkDescriptorSetLayout m_DShadowMapAlbedoDescriptorSetLayout =
						ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createDescriptorSetLayout({ 0 }, {}, { {1} });
					return m_DShadowMapAlbedoDescriptorSetLayout;
				}

				static ::std::vector<Vector2f> m_RecRawVertices;
				static VertexBuffer m_RecVertices;

				static ::std::vector<uint32_t> m_RecRawIndices;
				static IndexBuffer m_RecIndices;

				static BufferLayout m_RecLayout;

				static VertexArray m_RecVao;
		};
	}
}