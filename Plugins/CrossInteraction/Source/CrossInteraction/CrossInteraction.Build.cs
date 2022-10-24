// Some copyright should be here...

using UnrealBuildTool;
using System.IO;




public class CrossInteraction : ModuleRules
{


	public CrossInteraction(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"CrossInteraction/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"CrossInteraction/Private"
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "Messaging", "RenderCore", "RHI"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"Json",
				"JsonUtilities",
				"Networking",
				"Sockets",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);



		LoadOpenCV(Target);
	}
	
	private string ThirdPartyDirectory
	{ get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty")); } }

	public bool LoadOpenCV(ReadOnlyTargetRules Target)
	{
		//���OpenCV��������ĸ�·��
		string OpenCVPath = Path.Combine(ThirdPartyDirectory, "OpenCV");
		//Lib�ļ�·��
		string LibPath = "";
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			//���潫��include���ļ����µĵ������⣬��.hpp�ļ�.
			PublicIncludePaths.AddRange(new string[] { Path.Combine(OpenCVPath, "Includes") });

			//ָ��ThirdpartyPath/Libraries/Win64/
			LibPath = Path.Combine(OpenCVPath, "Libraries", "Win64");

			//�⽫��������lib�ļ���·��
			PublicSystemLibraryPaths.Add(LibPath);
			//�⽫����������Ҫ���ص�lib�ļ������ƣ�����ȥ�����lib�ļ�·����Ѱ�������ṩ��lib�ļ�
			PublicAdditionalLibraries.Add("opencv_world453.lib");

			/**
			 * dll��Ҫ��������Ŀ��Ŀ¼/Binaries/��ǰƽ̨/·����
			 *����ʱ�����������ڴ��ʱ���Զ���dll��Դ�ļ�·��������Ľ��Ŷ�ļ�·��
			 * RuntimeDependencies.Add(Դ�ļ�·����Ŀ���ļ�·����
			 * $(BinaryOutputDir),��ʾBinaries/��ǰƽ̨/·��
			 * �ο���https://docs.unrealengine.com/4.27/zh-CN/ProductionPipelines/BuildTools/UnrealBuildTool/ThirdPartyLibraries/
			 */
			RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "opencv_world453.dll"), Path.Combine(LibPath, "opencv_world453.dll"));
			RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "opencv_videoio_ffmpeg453_64.dll"), Path.Combine(LibPath, "opencv_videoio_ffmpeg453_64.dll"));
			return true;
		}
		return false;
	}
}
