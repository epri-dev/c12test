#include <MCOM/MCOMExtern.h>
#include <MCOM/MCOM.h>
#include "Setup.h"

const MStdString s_defaultIniFileName = "default.ini";
const MStdString s_defaultChannelProperties = "TYPE=CHANNEL_OPTICAL_PROBE";
const MStdString s_defaultProtocolProperties = "TYPE=PROTOCOL_ANSI_C12_18";

Setup::Setup()
:
   m_protocol(nullptr),
   m_channel(nullptr),
   m_tables(),
   m_verbose(false),
   m_single(false),
   m_fullauto(false)
{
}

Setup::~Setup()
{
   delete m_protocol;
   delete m_channel;
}

bool Setup::Initialize(int argc, char** argv)
{
   MCommandLineParser parser;

   MStdString channelProperties  = s_defaultChannelProperties;
   MStdString protocolProperties = s_defaultProtocolProperties;
   MStdString iniFileName        = s_defaultIniFileName;
#if !M_NO_MCOM_MONITOR
   MStdString monitorFileName;
   MStdString monitorAddress;
#endif
   try
   {
      parser.SetDescription("C12 table reader " M_PRODUCT_LEGAL_COPYRIGHT);
      parser.SetCopyright("Metering SDK " M_SDK_VERSION_STRING " " M_SDK_COPYRIGHT);
      parser.DeclareNamedString('c', "channel",  "properties", "Channel properties",  channelProperties);
      parser.DeclareNamedString('p', "protocol", "properties", "Protocol properties", protocolProperties);
      parser.DeclareNamedString('C', "config",   "file-name",  "Configuration file name",              iniFileName);
      parser.DeclareFlag('v', "verbose", "Full diagnostic output", m_verbose);
      parser.DeclareFlag('s', "single", "Read single tables, skipping over errors", m_single);
      parser.DeclareFlag('A', "automatic", "Fully automatic mode", m_fullauto);
#if !M_NO_MCOM_MONITOR
      parser.DeclareNamedString('f', "monitor-file",    "file-name", "Store communication log to ml file", monitorFileName);
      parser.DeclareNamedString('a', "monitor-address", "file-name", "Send monitor data to this address", monitorAddress);
#endif
      parser.DeclareStringVector("tables-functions", "Tables to read and/or functions to execute", m_tables);
      parser.SetFooter("Channel properties example:\n"
                       "    TYPE=CHANNEL_SOCKET;PEER_ADDRESS=10.0.0.123;PEER_PORT=80\n"
                       "Protocol properties example:\n"
                       "    TYPE=PROTOCOL_ANSI_C12_21;IDENTITY=2\n"
                       "Tables-functions can be a list of the following items:\n"
                       "  - Table reads by number, such as: 1 5 2049\n"
                       "  - Table reads: ST1 ST5\n"
                       "  - Function with no request: SF3()\n"
                       "  - Function with request: SF4(01)\n"
                       "Table 1 will always be read at the start.\n"
                       "Do not forget to enclose semicolons and blanks with quotes.\n");
      if ( parser.Process(argc, argv) != 0 )
         return false;

      parser.WriteHeader();

      if ( iniFileName != s_defaultIniFileName || MUtilities::IsPathExisting(iniFileName) ) // default.ini can be absent but any other ini can not
         DoReadIni(iniFileName);

      if ( m_protocol == nullptr )
         m_protocol = MCOMFactory::CreateProtocol(MVariant(MVariant::VAR_OBJECT), protocolProperties);
      else if ( protocolProperties != s_defaultProtocolProperties )
         m_protocol->SetPersistentPropertyValues(protocolProperties);

      if ( m_channel == nullptr )
         m_channel = MCOMFactory::CreateChannel(channelProperties);
      else if ( channelProperties != s_defaultChannelProperties )
         m_channel->SetPersistentPropertyValues(channelProperties);

      m_protocol->SetIsChannelOwned(false);
      m_protocol->SetChannel(m_channel);

#if !M_NO_MCOM_MONITOR
      MMonitorFile* monitor = nullptr;
      if ( !monitorAddress.empty() )
         monitor = M_NEW MMonitorSocket(monitorAddress);
      if ( !monitorFileName.empty() )
      {
         if ( monitor == nullptr )
            monitor = M_NEW MMonitorFile(monitorFileName);
         else
            monitor->SetFileName(monitorFileName);
      }
      if ( monitor != nullptr )
         m_channel->SetMonitor(monitor);
#endif
   }
   catch ( MException& ex )
   {
      parser.WriteException(ex);
      return false;
   }

   return true;
}

void Setup::DoReadIni(const std::string& fileName)
{
   MIniFile iniFile(fileName, false);
   DoReadIniDetermineTypes(iniFile);
   iniFile.ReInit();
   DoReadIniPopulateValues(iniFile);
}

void Setup::DoReadIniDetermineTypes(MIniFile& iniFile)
{
   enum ParsingType
   {
      ParsingNone,
      ParsingChannel,
      ParsingProtocol
   };
   ParsingType parsing = ParsingNone;

   for ( ;; )   // First pass through the ini, determine protocol types
   {
      MIniFile::LineType type = iniFile.ReadLine();
      if ( type == MIniFile::LineEof )
         break;
      if ( type == MIniFile::LineKey )
      {
         const MStdString& key = iniFile.GetKey();
         if ( key == "protocol" )
            parsing = ParsingProtocol;
         else if ( key == "channel" )
            parsing = ParsingChannel;
         else
         {
            iniFile.ThrowError("Keys expected are only [protocol] or [channel], case sensitive");
            M_ENSURED_ASSERT(0);
         }
      }
      else if ( parsing != ParsingNone && type == MIniFile::LineNameValue )
      {
         const MStdString& name = iniFile.GetName();
         if ( name == "TYPE" || name == "Type" )
         {
            if ( parsing == ParsingProtocol )
            {
               if ( m_protocol != nullptr )
               {
                  iniFile.ThrowError("Duplicate Type/Configuration value for protocol");
                  M_ENSURED_ASSERT(0);
               }
               m_protocol = MCOMFactory::CreateProtocolByName(nullptr, iniFile.GetStringValue());
            }
            else
            {
               M_ASSERT(parsing == ParsingChannel);
               if ( m_channel != nullptr )
               {
                  iniFile.ThrowError("Duplicate Type/Configuration value for protocol");
                  M_ENSURED_ASSERT(0);
               }
               m_channel = MCOMFactory::CreateChannelByName(iniFile.GetStringValue());
            }
         }
         else if ( name == "CONFIGURATION" || name == "Configuration" )
         {
            if ( parsing == ParsingProtocol )
            {
               if ( m_protocol != nullptr )
               {
                  iniFile.ThrowError("Duplicate Type/Configuration value for protocol");
                  M_ENSURED_ASSERT(0);
               }
               m_protocol = MCOMFactory::CreateProtocol(MVariant(MVariant::VAR_OBJECT), iniFile.GetStringValue());
            }
            else
            {
               M_ASSERT(parsing == ParsingChannel);
               if ( m_channel != nullptr )
               {
                  iniFile.ThrowError("Duplicate Type/Configuration value for protocol");
                  M_ENSURED_ASSERT(0);
               }
               m_channel = MCOMFactory::CreateChannel(iniFile.GetStringValue());
            }
         }
      }
   }
}

void Setup::DoReadIniPopulateValues(MIniFile& iniFile)
{
   MCOMObject* obj = nullptr;
   for ( ;; )    // Second pass, collect properties
   {
      MIniFile::LineType type = iniFile.ReadLine();
      if ( type == MIniFile::LineEof )
         break;
      if ( type == MIniFile::LineKey )
      {
         const MStdString& key = iniFile.GetKey();
         if ( key == "protocol" )
            obj = m_protocol;
         else if ( key == "channel" )
            obj = m_channel;
         else
         {
            M_ASSERT(0); // this condition was already reported
         }
      }
      else if ( obj != nullptr && type == MIniFile::LineNameValue )
      {
         const MStdString& name = iniFile.GetName();
         if ( name != "CONFIGURATION" && name != "Configuration" )
            obj->SetProperty(name, iniFile.GetValue());
      }
   }
}
