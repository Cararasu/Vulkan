##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=VulkanTest
ConfigurationName      :=Debug
WorkspacePath          :=/home/thomas/Prog/holodec
ProjectPath            :=/home/thomas/Prog/Vulkan
IntermediateDirectory  :=./DebugWind
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=thomas
Date                   :=07/11/17
CodeLitePath           :=/home/thomas/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
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
ObjectsFileList        :="VulkanTest.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)/home/thomas/VulkanSDK/1.0.61.1/x86_64/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)glfw3 $(LibrarySwitch)vulkan $(LibrarySwitch)X11 $(LibrarySwitch)Xi $(LibrarySwitch)Xrandr $(LibrarySwitch)Xxf86vm $(LibrarySwitch)Xinerama $(LibrarySwitch)Xcursor $(LibrarySwitch)rt $(LibrarySwitch)pthread $(LibrarySwitch)dl 
ArLibs                 :=  "glfw3" "vulkan" "X11" "Xi" "Xrandr" "Xxf86vm" "Xinerama" "Xcursor" "rt" "pthread" "dl" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)/home/thomas/VulkanSDK/1.0.61.1/x86_64/lib 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -Wall -std=c++11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) $(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) $(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) $(IntermediateDirectory)/RenderPasses.cpp$(ObjectSuffix) $(IntermediateDirectory)/Shader.cpp$(ObjectSuffix) $(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./DebugWind || $(MakeDirCommand) ./DebugWind


$(IntermediateDirectory)/.d:
	@test -d ./DebugWind || $(MakeDirCommand) ./DebugWind

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix): VHeader.cpp $(IntermediateDirectory)/VHeader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/VHeader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VHeader.cpp$(DependSuffix): VHeader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VHeader.cpp$(DependSuffix) -MM VHeader.cpp

$(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix): VHeader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix) VHeader.cpp

$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix): VGlobal.cpp $(IntermediateDirectory)/VGlobal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/VGlobal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix): VGlobal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix) -MM VGlobal.cpp

$(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix): VGlobal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix) VGlobal.cpp

$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix): VDevice.cpp $(IntermediateDirectory)/VDevice.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/VDevice.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VDevice.cpp$(DependSuffix): VDevice.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VDevice.cpp$(DependSuffix) -MM VDevice.cpp

$(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix): VDevice.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix) VDevice.cpp

$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix): VWindow.cpp $(IntermediateDirectory)/VWindow.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/VWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VWindow.cpp$(DependSuffix): VWindow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VWindow.cpp$(DependSuffix) -MM VWindow.cpp

$(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix): VWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix) VWindow.cpp

$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix): ShaderInputAbstraction.cpp $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/ShaderInputAbstraction.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix): ShaderInputAbstraction.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(DependSuffix) -MM ShaderInputAbstraction.cpp

$(IntermediateDirectory)/ShaderInputAbstraction.cpp$(PreprocessSuffix): ShaderInputAbstraction.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ShaderInputAbstraction.cpp$(PreprocessSuffix) ShaderInputAbstraction.cpp

$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix): Pipelines.cpp $(IntermediateDirectory)/Pipelines.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/Pipelines.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Pipelines.cpp$(DependSuffix): Pipelines.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Pipelines.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Pipelines.cpp$(DependSuffix) -MM Pipelines.cpp

$(IntermediateDirectory)/Pipelines.cpp$(PreprocessSuffix): Pipelines.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Pipelines.cpp$(PreprocessSuffix) Pipelines.cpp

$(IntermediateDirectory)/RenderPasses.cpp$(ObjectSuffix): RenderPasses.cpp $(IntermediateDirectory)/RenderPasses.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/RenderPasses.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/RenderPasses.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/RenderPasses.cpp$(DependSuffix): RenderPasses.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/RenderPasses.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/RenderPasses.cpp$(DependSuffix) -MM RenderPasses.cpp

$(IntermediateDirectory)/RenderPasses.cpp$(PreprocessSuffix): RenderPasses.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/RenderPasses.cpp$(PreprocessSuffix) RenderPasses.cpp

$(IntermediateDirectory)/Shader.cpp$(ObjectSuffix): Shader.cpp $(IntermediateDirectory)/Shader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/Shader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Shader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Shader.cpp$(DependSuffix): Shader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Shader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Shader.cpp$(DependSuffix) -MM Shader.cpp

$(IntermediateDirectory)/Shader.cpp$(PreprocessSuffix): Shader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Shader.cpp$(PreprocessSuffix) Shader.cpp

$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix): Buffer.cpp $(IntermediateDirectory)/Buffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/Buffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Buffer.cpp$(DependSuffix): Buffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Buffer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Buffer.cpp$(DependSuffix) -MM Buffer.cpp

$(IntermediateDirectory)/Buffer.cpp$(PreprocessSuffix): Buffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Buffer.cpp$(PreprocessSuffix) Buffer.cpp

$(IntermediateDirectory)/Image.cpp$(ObjectSuffix): Image.cpp $(IntermediateDirectory)/Image.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/Image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Image.cpp$(DependSuffix): Image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Image.cpp$(DependSuffix) -MM Image.cpp

$(IntermediateDirectory)/Image.cpp$(PreprocessSuffix): Image.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Image.cpp$(PreprocessSuffix) Image.cpp

$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix): DataWrapper.cpp $(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/thomas/Prog/Vulkan/DataWrapper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix): DataWrapper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DataWrapper.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DataWrapper.cpp$(DependSuffix) -MM DataWrapper.cpp

$(IntermediateDirectory)/DataWrapper.cpp$(PreprocessSuffix): DataWrapper.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DataWrapper.cpp$(PreprocessSuffix) DataWrapper.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./DebugWind/


