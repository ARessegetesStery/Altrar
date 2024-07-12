#pragma once
#include "ATRType.h"

namespace ATR
{
	enum class ExceptionType
	{
		INIT
	};

	class Exception
	{
	public:
		Exception(const String& msg, const ExceptionType type) : msg(msg), type(type) { }
	
		const String& What() const { return this->msg; }
		const ExceptionType& Type() const { return this->type; }
		const String& Msg() const
		{
			String typeStr = "";
			switch (this->type)
			{
			case ExceptionType::INIT:
				typeStr += "[INIT]";
			}

			return typeStr + " " + this->msg;
		}
	
	private:
		String msg;
		ExceptionType type;
	};

}
