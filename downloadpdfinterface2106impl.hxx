/* 
 @<COPYRIGHT>@
 ==================================================
 Copyright 2012
 Siemens Product Lifecycle Management Software Inc.
 All Rights Reserved.
 ==================================================
 @<COPYRIGHT>@
*/

#ifndef TEAMCENTER_SERVICES_CUSTOMSERVICE_2021_06_DOWNLOADPDFINTERFACE_IMPL_HXX 
#define TEAMCENTER_SERVICES_CUSTOMSERVICE_2021_06_DOWNLOADPDFINTERFACE_IMPL_HXX


#include <downloadpdfinterface2106.hxx>

#include <CustomService_exports.h>

namespace SK2
{
    namespace Soa
    {
        namespace CustomService
        {
            namespace _2021_06
            {
                class DownloadPDFInterfaceImpl;
            }
        }
    }
}


class SOACUSTOMSERVICE_API SK2::Soa::CustomService::_2021_06::DownloadPDFInterfaceImpl : public SK2::Soa::CustomService::_2021_06::DownloadPDFInterface

{
public:

    virtual std::string downloadCOPDF ( const std::string input );
    virtual std::string downloadAssemblyPDF ( const std::string input );


};

#include <CustomService_undef.h>
#endif
