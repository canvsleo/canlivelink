using UnrealBuildTool;
using System.Collections.Generic;

public class MaxLiveLinkPlugin2018Target
	: MaxLiveLinkPluginTargetBase
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

	public MaxLiveLinkPlugin2018Target(TargetInfo Target) 
		: base(Target)
	{
		LaunchModuleName = 
			"MaxLiveLinkPlugin" + this.Get3dsMaxVersion()
			;
	}
}
