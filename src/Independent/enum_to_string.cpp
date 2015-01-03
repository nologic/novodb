//
//  enum_to_string.cpp
//  Novodb
//
//  Created by mike on 12/29/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "enum_to_string.h"

namespace novo {
    std::string state_type_to_string(lldb::StateType state) {
        switch(state) {
            case lldb::eStateInvalid:   return std::string("invalid");
            case lldb::eStateUnloaded:  return std::string("unloaded");
            case lldb::eStateConnected: return std::string("connected");
            case lldb::eStateAttaching: return std::string("attaching");
            case lldb::eStateLaunching: return std::string("launching");
            case lldb::eStateStopped:   return std::string("stopped");
            case lldb::eStateRunning:   return std::string("running");
            case lldb::eStateStepping:  return std::string("stepping");
            case lldb::eStateCrashed:   return std::string("crashed");
            case lldb::eStateDetached:  return std::string("detached");
            case lldb::eStateExited:    return std::string("exited");
            case lldb::eStateSuspended: return std::string("suspended");
            default: return std::string("Unknown");
        }
    }
    
    std::string address_class_to_string(lldb::AddressClass cl) {
        switch(cl) {
            case lldb::eAddressClassInvalid: return "Invalid";
            case lldb::eAddressClassUnknown: return "Unknown";
            case lldb::eAddressClassCode: return "Code";
            case lldb::eAddressClassCodeAlternateISA: return "CodeAlternateISA";
            case lldb::eAddressClassData: return "Data";
            case lldb::eAddressClassDebug: return "Debug";
            case lldb::eAddressClassRuntime: return "Runtime";
            default: return "Unknown";
        }
    }
    
    std::string section_type_to_string(lldb::SectionType st) {
        using namespace lldb;
        
        switch (st) {
            case eSectionTypeInvalid: return "Invalid";
            case eSectionTypeCode: return "Code";
            case eSectionTypeContainer: return "Container";
            case eSectionTypeData: return "Data";
            case eSectionTypeDataCString: return "DataCString";
            case eSectionTypeDataCStringPointers: return "DataCStringPointers";
            case eSectionTypeDataSymbolAddress: return "DataSymbolAddress";
            case eSectionTypeData4: return "Data4";
            case eSectionTypeData8: return "Data8";
            case eSectionTypeData16: return "Data16";
            case eSectionTypeDataPointers: return "DataPointers";
            case eSectionTypeDebug: return "Debug";
            case eSectionTypeZeroFill: return "ZeroFill";
            case eSectionTypeDataObjCMessageRefs: return "DataObjCMessageRefs";
            case eSectionTypeDataObjCCFStrings: return "DataObjCCFStrings";
            case eSectionTypeDWARFDebugAbbrev: return "DWARFDebugAbbrev";
            case eSectionTypeDWARFDebugAranges: return "DWARFDebugAranges";
            case eSectionTypeDWARFDebugFrame: return "DWARFDebugFrame";
            case eSectionTypeDWARFDebugInfo: return "DWARFDebugInfo";
            case eSectionTypeDWARFDebugLine: return "DWARFDebugLine";
            case eSectionTypeDWARFDebugLoc: return "DWARFDebugLoc";
            case eSectionTypeDWARFDebugMacInfo: return "DWARFDebugMacInfo";
            case eSectionTypeDWARFDebugPubNames: return "DWARFDebugPubNames";
            case eSectionTypeDWARFDebugPubTypes: return "DWARFDebugPubTypes";
            case eSectionTypeDWARFDebugRanges: return "DWARFDebugRanges";
            case eSectionTypeDWARFDebugStr: return "DWARFDebugStr";
            case eSectionTypeDWARFAppleNames: return "DWARFAppleNames";
            case eSectionTypeDWARFAppleTypes: return "DWARFAppleTypes";
            case eSectionTypeDWARFAppleNamespaces: return "DWARFAppleNamespaces";
            case eSectionTypeDWARFAppleObjC: return "DWARFAppleObjC";
            case eSectionTypeELFSymbolTable: return "ELFSymbolTable";
            case eSectionTypeELFDynamicSymbols: return "ELFDynamicSymbols";
            case eSectionTypeELFRelocationEntries: return "ELFRelocationEntries";
            case eSectionTypeELFDynamicLinkInfo: return "ELFDynamicLinkInfo";
            case eSectionTypeEHFrame: return "EHFrame";
            case eSectionTypeOther: return "Other";
            default: return std::string("Unknown");
        }
    }
    
    std::string symbol_type_to_string(lldb::SymbolType st) {
        switch (st) {
            case lldb::eSymbolTypeInvalid: return "Invalid";
            case lldb::eSymbolTypeAbsolute: return "Absolute";
            case lldb::eSymbolTypeCode: return "Code";
            case lldb::eSymbolTypeResolver: return "Resolver";
            case lldb::eSymbolTypeData: return "Data";
            case lldb::eSymbolTypeTrampoline: return "Trampoline";
            case lldb::eSymbolTypeRuntime: return "Runtime";
            case lldb::eSymbolTypeException: return "Exception";
            case lldb::eSymbolTypeSourceFile: return "SourceFile";
            case lldb::eSymbolTypeHeaderFile: return "HeaderFile";
            case lldb::eSymbolTypeObjectFile: return "ObjectFile";
            case lldb::eSymbolTypeCommonBlock: return "CommonBlock";
            case lldb::eSymbolTypeBlock: return "Block";
            case lldb::eSymbolTypeLocal: return "Local";
            case lldb::eSymbolTypeParam: return "Param";
            case lldb::eSymbolTypeVariable: return "Variable";
            case lldb::eSymbolTypeVariableType: return "VariableType";
            case lldb::eSymbolTypeLineEntry: return "LineEntry";
            case lldb::eSymbolTypeLineHeader: return "LineHeader";
            case lldb::eSymbolTypeScopeBegin: return "ScopeBegin";
            case lldb::eSymbolTypeScopeEnd: return "ScopeEnd";
            case lldb::eSymbolTypeAdditional: return "Additional";
            case lldb::eSymbolTypeCompiler: return "Compiler";
            case lldb::eSymbolTypeInstrumentation: return "Instrumentation";
            case lldb::eSymbolTypeUndefined: return "Undefined";
            case lldb::eSymbolTypeObjCClass: return "ObjCClass";
            case lldb::eSymbolTypeObjCMetaClass: return "ObjCMetaClass";
            case lldb::eSymbolTypeObjCIVar: return "ObjCIVar";
            case lldb::eSymbolTypeReExported: return "ReExported";
            default: return "Unknown";
        }
    }
}