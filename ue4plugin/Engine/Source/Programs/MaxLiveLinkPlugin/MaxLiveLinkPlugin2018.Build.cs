using UnrealBuildTool;


public class MaxLiveLinkPlugin2018 
	: MaxLiveLinkPluginBase
{
	public override string Get3dsMaxVersion()
	{
		return "2018";
	}
	public override string Get3dsMaxBasePath()
	{
		return string.Format(
			"C:\\Program Files\\Autodesk\\3ds Max {0}",
			this.Get3dsMaxVersion()
		);
	}

	public MaxLiveLinkPlugin2018(ReadOnlyTargetRules Target) 
		: base(Target)
	{
	}
}
