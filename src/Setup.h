#ifndef SETUP_H
#define SETUP_H

#include <MCOM/MCOM.h>

// Handle program parameters whether they appear from command line or configuration ini file
//
class Setup
{
public:

   Setup();
   ~Setup();

   bool Initialize(int argc, char** argv);

   // Called after Initialize to get the result protocol
   //
   MProtocol* GetProtocol()
   {
      M_ASSERT(m_protocol != nullptr); //  otherwise initialize failed and this method should not be called
      return m_protocol;
   }

   ///@{
   /// Called after Initialize to get which tables to read
   ///
   MStdStringVector& GetTableNames()
   {
      return m_tables;
   }
   const MStdStringVector& GetTableNames() const
   {
      return m_tables;
   }
   ///@}

   /// Called after Initialize to get the value of verbose flag
   ///
   bool GetVerboseFlag() const
   {
      return m_verbose;
   }

   /// Called after Initialize to get the value of single flag
   ///
   bool GetSingleFlag() const
   {
      return m_single;
   }

   /// Called after Initialize to get the value of single flag
   ///
   bool GetFullAutoFlag() const
   {
      return m_fullauto;
   }

private:

   void DoReadIni(const std::string& fileName);
   void DoReadIniDetermineTypes(MIniFile& iniFile);
   void DoReadIniPopulateValues(MIniFile& iniFile);

   MProtocol*       m_protocol;
   MChannel*        m_channel;
   MStdStringVector m_tables;
   bool             m_verbose;
   bool             m_single;
   bool             m_fullauto;
};

#endif // SETUP_H
