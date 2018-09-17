// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class LiveLinkExtendPreviewPlugin : ModuleRules
	{
		public LiveLinkExtendPreviewPlugin(ReadOnlyTargetRules Target) : base(Target)
		{

			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateIncludePaths.AddRange(
				new string[] {
					"/LiveLinkExtendPreviewPlugin/Private",
					"Engine/Plugins/Animation/LiveLink/Public",
				}
			);
			
			PublicIncludePaths.AddRange(
				new string[] {
					"Engine/Plugins/Animation/LiveLink/Public",
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "UnrealEd",
                    "Engine",
                    "Projects",
					"RenderCore",

                    "Messaging",
					"LiveLink",
                    "LiveLinkInterface",
					"LiveLinkMessageBusFramework",
                    "Persona",

					"Json"
				}
			);

			SetupModulePhysXAPEXSupport(Target);
			if(Target.bCompilePhysX && (Target.bBuildEditor || Target.bCompileAPEX))
			{
				DynamicallyLoadedModuleNames.Add("PhysXCooking");
			}

		}
	}
}
