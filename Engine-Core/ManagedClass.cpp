#include "Includes\Core\ManagedClass.h"


MClass::MClass(const char* name, const uint16& id) :
	m_name(name), m_id(id)
{

}

ManagedObject* MClass::NewObject(void* dst) const 
{
	return nullptr;
}

const MClass* MClass::GetParentClass() const 
{
	return nullptr;
}

bool MClass::IsChildOf(const MClass* other, const bool& trueIfIdentical) const
{
	if (this == other)
		return trueIfIdentical;

	const MClass* parent = GetParentClass();
	if (parent == nullptr)
		return false;
	else
		return parent->IsChildOf(other, true);
}

const MClass* ManagedObject::StaticClass()
{
	return nullptr;
}

const MClass* ManagedObject::ParentStaticClass()
{
	return nullptr;
}