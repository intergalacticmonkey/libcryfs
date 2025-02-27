#pragma once
#ifndef MESSMER_CRYFS_SRC_CONFIG_CRYCONFIGLOADER_H_
#define MESSMER_CRYFS_SRC_CONFIG_CRYCONFIGLOADER_H_

#include <cpp-utils/pointer/unique_ref.h>
#include <boost/filesystem.hpp>
#include <cpp-utils/either.h>
#include "CryConfigFile.h"
#include "CryCipher.h"
#include "CryConfigCreator.h"
#include "CryKeyProvider.h"

namespace cryfs {

class CryConfigLoader final {
public:
  // note: keyGenerator generates the inner (i.e. file system) key. keyProvider asks for the password and generates the outer (i.e. config file) key.
  CryConfigLoader(cpputils::RandomGenerator &keyGenerator, cpputils::unique_ref<CryKeyProvider> keyProvider, LocalStateDir localStateDir, const boost::optional<std::string> &cipherFromCommandLine, const boost::optional<uint32_t> &blocksizeBytesFromCommandLine, const boost::optional<bool> &missingBlockIsIntegrityViolationFromCommandLine);
  CryConfigLoader(CryConfigLoader &&rhs) = default;

  struct ConfigLoadResult {
      CryConfig oldConfig; // loading a config file updates the config file, but this member keeps the original config
      cpputils::unique_ref<CryConfigFile> configFile;
      uint32_t myClientId;
  };

  cpputils::either<CryConfigFile::LoadError, CryConfigLoader::ConfigLoadResult> loadOrCreate(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem);
  cpputils::either<CryConfigFile::LoadError, CryConfigLoader::ConfigLoadResult> load(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem, CryConfigFile::Access access);
  boost::optional<CryConfigFile::LoadError> changeEncryptionKey(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem, cpputils::unique_ref<CryKeyProvider> newKeyProvider);

private:
    cpputils::either<CryConfigFile::LoadError, CryConfigLoader::ConfigLoadResult> _loadConfig(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem, CryConfigFile::Access access);
    ConfigLoadResult _createConfig(boost::filesystem::path filename, bool allowReplacedFilesystem);
    void _checkVersion(const CryConfig &config, bool allowFilesystemUpgrade);
    void _checkCipher(const CryConfig &config) const;
    void _checkMissingBlocksAreIntegrityViolations(CryConfigFile *configFile, uint32_t myClientId);

    CryConfigCreator _creator;
    cpputils::unique_ref<CryKeyProvider> _keyProvider;
    boost::optional<std::string> _cipherFromCommandLine;
    boost::optional<uint32_t> _blocksizeBytesFromCommandLine;
    boost::optional<bool> _missingBlockIsIntegrityViolationFromCommandLine;
    LocalStateDir _localStateDir;

    DISALLOW_COPY_AND_ASSIGN(CryConfigLoader);
};

}

#endif
