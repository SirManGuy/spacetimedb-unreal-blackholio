#pragma once
#include "CoreMinimal.h"
#include "FBinaryReader.h"
#include "FBinaryWriter.h"

struct SPACETIMEDB_API FQueryId
{
	uint32 Id;

	FQueryId()
		: Id(0)
	{
	}

	FQueryId(uint32 InId)
		: Id(InId)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		Id = reader.ReadUInt32();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(Id);
	}
};