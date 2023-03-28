#include "LayerStack.h"

namespace vgl
{
	LayerStack::LayerStack() : m_LayerInsert(m_Layers.begin())
	{

	}
	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers) {
			layer->onDetach();
			delete layer;
		}
	}

	void LayerStack::pushLayer(Layer* p_Layer)
	{
		p_Layer->onAttach();
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, p_Layer);
	}
	void LayerStack::pushOverlay(Layer* p_Overlay)
	{
		p_Overlay->onAttach();
		m_Layers.emplace_back(p_Overlay);
	}
	void LayerStack::popLayer(Layer* p_Layer)
	{
		auto it = std::find(begin(), end(), p_Layer);
		if (it != end())
		{
			(*it)->onDetach();
			m_Layers.erase(it);
			m_LayerInsert--;
		}
	}
	void LayerStack::popOverlay(Layer* p_Overlay)
	{
		auto it = std::find(begin(), end(), p_Overlay);
		if (it != end()) {
			(*it)->onDetach();
			m_Layers.erase(it);
		}
	}
}