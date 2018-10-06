using UnrealBuildTool;


public class MaxLiveLinkPlugin2019 
	: MaxLiveLinkPluginBase
{
	public override string Get3dsMaxVersion()
	{
		return "2019";
	}
	public override string Get3dsMaxBasePath()
	{
		return string.Format(
			"C:\\Program Files\\Autodesk\\3ds Max {0}",
			this.Get3dsMaxVersion()
		);
	}

	public MaxLiveLinkPlugin2019(ReadOnlyTargetRules Target) 
		: base(Target)
	{
	}
}
