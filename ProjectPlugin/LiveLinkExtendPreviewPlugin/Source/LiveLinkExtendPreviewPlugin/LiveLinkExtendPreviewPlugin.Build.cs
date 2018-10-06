
namespace UnrealBuildTool.Rules
{
	public class LiveLinkExtendPreviewPlugin : ModuleRules
	{
		public LiveLinkExtendPreviewPlugin(ReadOnlyTargetRules Target) : base(Target)
		{

			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateIncludePaths.AddRange(
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
		}
	}
}
