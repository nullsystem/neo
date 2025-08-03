#pragma once

class CNEOBotContextualQueryInterface
{
public:
	~CNEOBotContextualQueryInterface() {}

	// Should the bot walk?
	virtual QueryResultType ShouldWalk(const INextBot *me) const = 0;
};

