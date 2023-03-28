#pragma once

#include <iostream>
#include <vector>

#include "Layer.h"

namespace vgl
{
	class LayerStack
	{
		public:
			LayerStack();
			~LayerStack();

			void pushLayer(Layer* p_Layer);
			void pushOverlay(Layer* p_Overlay);
			void popLayer(Layer* p_Layer);
			void popOverlay(Layer* p_Overlay);

			std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
			std::vector<Layer*>::iterator end() { return m_Layers.end(); }

		private:
			friend class Application;
			std::vector<Layer*> m_Layers;
			std::vector<Layer*>::iterator m_LayerInsert;
	};
}