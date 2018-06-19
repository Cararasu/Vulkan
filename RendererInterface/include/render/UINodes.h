#pragma once

struct UINode {
	
	//on_event(event, ptr)
};
struct BorderNode : public UINode{
	virtual RendResult center(UINode*) = 0;
	
	virtual RendResult top(UINode*) = 0;
	virtual RendResult bottom(UINode*) = 0;
	
	virtual RendResult left(UINode*) = 0;
	virtual RendResult right(UINode*) = 0;
	
	virtual UINode* center() = 0;
	
	virtual UINode* top() = 0;
	virtual UINode* bottom() = 0;
	
	virtual UINode* left() = 0;
	virtual UINode* right() = 0;
	
};
enum class AccordionDirection{
	eUp,
	eDown,
	eRight,
	eLeft
};
struct AccordionNode : public UINode {
	
	virtual RendResult direction(AccordionDirection) = 0;
	virtual AccordionDirection direction() = 0;
};