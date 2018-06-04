##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug_Windows
ProjectName            :=Vulkan
ConfigurationName      :=Debug_Windows
WorkspacePath          :=E:/GNUProg/Vulkan
ProjectPath            :=E:/GNUProg/Vulkan/Vulkan
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Thomas
Date                   :=04/06/2018
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :="C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/g++.exe"
SharedObjectLinkerName :="C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/g++.exe" -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Vulkan.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :="C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/windres.exe"
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../libs/glfw-3.2.1.bin.WIN64\include $(IncludeSwitch)../libs/glm-0.9.9-a1 $(IncludeSwitch)$(VULKAN_PATH)\Include $(IncludeSwitch)../libs/stb $(IncludeSwitch)../RendererInterface/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)glfw3 $(LibrarySwitch)gdi32 $(LibrarySwitch)user32 $(LibrarySwitch)kernel32 $(LibrarySwitch)vulkan-1 
ArLibs                 :=  "glfw3" "gdi32" "user32" "kernel32" "vulkan-1" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../libs/glfw-3.2.1.bin.WIN64\lib-mingw-w64 $(LibraryPathSwitch)$(VULKAN_PATH)\Lib 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/ar.exe" rcu
CXX      := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/g++.exe"
CC       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/gcc.exe"
CXXFLAGS := -Wall -std=c++17 -g -O0 -Wall $(Preprocessors)
CFLAGS   := -Wall -std=c99 -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/as.exe"


##
## User defined environment variables
##
VULKAN_PATH:="C:\Program Files\VulkanSDK\1.0.61.1"
Objects0=$(IntermediateDirectory)/vulkanmain.cpp$(ObjectSuffix) $(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) $(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) $(IntermediateDirectory)/DescriptorSet.cpp$(ObjectSuffix) $(IntermediateDirectory)/DrawDispatcher.cpp$(ObjectSuffix) $(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IntermediateDirectory)/PipelineModule.cpp$(ObjectSuffix) $(IntermediateDirectory)/Instance.cpp$(ObjectSuffix) $(IntermediateDirectory)/RenderEnvironment.cpp$(ObjectSuffix) $(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Dispatcher.cpp$(ObjectSuffix) $(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) $(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IntermediateDirectory)/VInstance.cpp$(ObjectSuffix) $(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) $(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d "..\.build-debug_windows\MetaBuilder" $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

"..\.build-debug_windows\MetaBuilder":
	@$(MakeDirCommand) "..\.build-debug_windows"
	@echo stam > "..\.build-debug_windows\MetaBuilder"




MakeIntermediateDirs:
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/vulkanmain.cpp$(ObjectSuffix): vulkanmain.cpp $(IntermediateDirectory)/vulkanmain.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/vulkanmain.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/vulkanmain.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/vulkanmain.cpp$(DependSuffix): vulkanmain.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/vulkanmain.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/vulkanmain.cpp$(DependSuffix) -MM vulkanmain.cpp

$(IntermediateDirectory)/vulkanmain.cpp$(PreprocessSuffix): vulkanmain.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/vulkanmain.cpp$(PreprocessSuffix) vulkanmain.cpp

$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix): Buffer.cpp $(IntermediateDirectory)/Buffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/Buffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Buffer.cpp$(DependSuffix): Buffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Buffer.cpp$(DependSuffix) -MM Buffer.cpp

$(IntermediateDirectory)/Buffer.cpp$(PreprocessSuffix): Buffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Buffer.cpp$(PreprocessSuffix) Buffer.cpp

$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix): DataWrapper.cpp $(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/DataWrapper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix): DataWrapper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix) -MM DataWrapper.cpp

$(IntermediateDirectory)/DataWrapper.cpp$(PreprocessSuffix): DataWrapper.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DataWrapper.cpp$(PreprocessSuffix) DataWrapper.cpp

$(IntermediateDirectory)/DescriptorSet.cpp$(ObjectSuffix): DescriptorSet.cpp $(IntermediateDirectory)/DescriptorSet.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/DescriptorSet.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DescriptorSet.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DescriptorSet.cpp$(DependSuffix): DescriptorSet.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DescriptorSet.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DescriptorSet.cpp$(DependSuffix) -MM DescriptorSet.cpp

$(IntermediateDirectory)/DescriptorSet.cpp$(PreprocessSuffix): DescriptorSet.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DescriptorSet.cpp$(PreprocessSuffix) DescriptorSet.cpp

$(IntermediateDirectory)/DrawDispatcher.cpp$(ObjectSuffix): DrawDispatcher.cpp $(IntermediateDirectory)/DrawDispatcher.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/DrawDispatcher.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DrawDispatcher.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DrawDispatcher.cpp$(DependSuffix): DrawDispatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DrawDispatcher.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DrawDispatcher.cpp$(DependSuffix) -MM DrawDispatcher.cpp

$(IntermediateDirectory)/DrawDispatcher.cpp$(PreprocessSuffix): DrawDispatcher.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DrawDispatcher.cpp$(PreprocessSuffix) DrawDispatcher.cpp

$(IntermediateDirectory)/Image.cpp$(ObjectSuffix): Image.cpp $(IntermediateDirectory)/Image.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/Image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Image.cpp$(DependSuffix): Image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Image.cpp$(DependSuffix) -MM Image.cpp

$(IntermediateDirectory)/Image.cpp$(PreprocessSuffix): Image.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Image.cpp$(PreprocessSuffix) Image.cpp

$(IntermediateDirectory)/PipelineModule.cpp$(ObjectSuffix): PipelineModule.cpp $(IntermediateDirectory)/PipelineModule.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/PipelineModule.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/PipelineModule.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/PipelineModule.cpp$(DependSuffix): PipelineModule.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/PipelineModule.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/PipelineModule.cpp$(DependSuffix) -MM PipelineModule.cpp

$(IntermediateDirectory)/PipelineModule.cpp$(PreprocessSuffix): PipelineModule.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/PipelineModule.cpp$(PreprocessSuffix) PipelineModule.cpp

$(IntermediateDirectory)/Instance.cpp$(ObjectSuffix): Instance.cpp $(IntermediateDirectory)/Instance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/Instance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Instance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Instance.cpp$(DependSuffix): Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Instance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Instance.cpp$(DependSuffix) -MM Instance.cpp

$(IntermediateDirectory)/Instance.cpp$(PreprocessSuffix): Instance.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Instance.cpp$(PreprocessSuffix) Instance.cpp

$(IntermediateDirectory)/RenderEnvironment.cpp$(ObjectSuffix): RenderEnvironment.cpp $(IntermediateDirectory)/RenderEnvironment.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/RenderEnvironment.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/RenderEnvironment.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/RenderEnvironment.cpp$(DependSuffix): RenderEnvironment.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/RenderEnvironment.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/RenderEnvironment.cpp$(DependSuffix) -MM RenderEnvironment.cpp

$(IntermediateDirectory)/RenderEnvironment.cpp$(PreprocessSuffix): RenderEnvironment.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/RenderEnvironment.cpp$(PreprocessSuffix) RenderEnvironment.cpp

$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix): VGlobal.cpp $(IntermediateDirectory)/VGlobal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/VGlobal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix): VGlobal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix) -MM VGlobal.cpp

$(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix): VGlobal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix) VGlobal.cpp

$(IntermediateDirectory)/Dispatcher.cpp$(ObjectSuffix): Dispatcher.cpp $(IntermediateDirectory)/Dispatcher.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/Dispatcher.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Dispatcher.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Dispatcher.cpp$(DependSuffix): Dispatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Dispatcher.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Dispatcher.cpp$(DependSuffix) -MM Dispatcher.cpp

$(IntermediateDirectory)/Dispatcher.cpp$(PreprocessSuffix): Dispatcher.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Dispatcher.cpp$(PreprocessSuffix) Dispatcher.cpp

$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix): VWindow.cpp $(IntermediateDirectory)/VWindow.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/VWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VWindow.cpp$(DependSuffix): VWindow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VWindow.cpp$(DependSuffix) -MM VWindow.cpp

$(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix): VWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix) VWindow.cpp

$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix): VHeader.cpp $(IntermediateDirectory)/VHeader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/VHeader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VHeader.cpp$(DependSuffix): VHeader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VHeader.cpp$(DependSuffix) -MM VHeader.cpp

$(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix): VHeader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix) VHeader.cpp

$(IntermediateDirectory)/VInstance.cpp$(ObjectSuffix): VInstance.cpp $(IntermediateDirectory)/VInstance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/VInstance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VInstance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VInstance.cpp$(DependSuffix): VInstance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VInstance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VInstance.cpp$(DependSuffix) -MM VInstance.cpp

$(IntermediateDirectory)/VInstance.cpp$(PreprocessSuffix): VInstance.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VInstance.cpp$(PreprocessSuffix) VInstance.cpp

$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix): VDevice.cpp $(IntermediateDirectory)/VDevice.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/VDevice.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VDevice.cpp$(DependSuffix): VDevice.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VDevice.cpp$(DependSuffix) -MM VDevice.cpp

$(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix): VDevice.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix) VDevice.cpp

$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix): ShaderInputAbstraction.cpp $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/ShaderInputAbstraction.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix): ShaderInputAbstraction.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix) -MM ShaderInputAbstraction.cpp

$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(PreprocessSuffix): ShaderInputAbstraction.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(PreprocessSuffix) ShaderInputAbstraction.cpp

$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix): Pipelines.cpp $(IntermediateDirectory)/Pipelines.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/Vulkan/Vulkan/Pipelines.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Pipelines.cpp$(DependSuffix): Pipelines.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Pipelines.cpp$(DependSuffix) -MM Pipelines.cpp

$(IntermediateDirectory)/Pipelines.cpp$(PreprocessSuffix): Pipelines.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Pipelines.cpp$(PreprocessSuffix) Pipelines.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


