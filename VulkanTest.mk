##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=VulkanTest
ConfigurationName      :=Debug
WorkspacePath          :=E:/GNUProg/holodec
ProjectPath            :=E:/GNUProg/VulkanTest
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Thomas
Date                   :=23/10/2017
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
ObjectsFileList        :="VulkanTest.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :="C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/windres.exe"
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)glfw-3.2.1.bin.WIN64\include $(IncludeSwitch)glm-0.9.9-a1 $(IncludeSwitch)"C:\Program Files\VulkanSDK\1.0.61.1\Include" 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)glfw3 $(LibrarySwitch)user32 $(LibrarySwitch)kernel32 $(LibrarySwitch)gdi32 $(LibrarySwitch)vulkan-1 
ArLibs                 :=  "glfw3" "user32" "kernel32" "gdi32" "vulkan-1" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)glfw-3.2.1.bin.WIN64\lib-mingw-w64 $(LibraryPathSwitch)"C:\Program Files\VulkanSDK\1.0.61.1\Lib" 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/ar.exe" rcu
CXX      := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/g++.exe"
CC       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/gcc.exe"
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := "C:/Program Files/mingw-w64/x86_64-7.1.0-posix-seh-rt_v5-rev2/mingw64/bin/as.exe"


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) $(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) 



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
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/VulkanTest/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix): VHeader.cpp $(IntermediateDirectory)/VHeader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/VulkanTest/VHeader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VHeader.cpp$(DependSuffix): VHeader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VHeader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VHeader.cpp$(DependSuffix) -MM VHeader.cpp

$(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix): VHeader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VHeader.cpp$(PreprocessSuffix) VHeader.cpp

$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix): VGlobal.cpp $(IntermediateDirectory)/VGlobal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/VulkanTest/VGlobal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix): VGlobal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VGlobal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VGlobal.cpp$(DependSuffix) -MM VGlobal.cpp

$(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix): VGlobal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VGlobal.cpp$(PreprocessSuffix) VGlobal.cpp

$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix): VDevice.cpp $(IntermediateDirectory)/VDevice.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/VulkanTest/VDevice.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VDevice.cpp$(DependSuffix): VDevice.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VDevice.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VDevice.cpp$(DependSuffix) -MM VDevice.cpp

$(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix): VDevice.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VDevice.cpp$(PreprocessSuffix) VDevice.cpp

$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix): VWindow.cpp $(IntermediateDirectory)/VWindow.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/GNUProg/VulkanTest/VWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/VWindow.cpp$(DependSuffix): VWindow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/VWindow.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/VWindow.cpp$(DependSuffix) -MM VWindow.cpp

$(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix): VWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/VWindow.cpp$(PreprocessSuffix) VWindow.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


