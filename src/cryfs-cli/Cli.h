#pragma once
#ifndef MESSMER_CRYFSCLI_CLI_H
#define MESSMER_CRYFSCLI_CLI_H

#include <fspp/fuse/Fuse.h>
#include "program_options/ProgramOptions.h"
#include <cryfs/impl/config/CryConfigFile.h>
#include <cryfs/impl/config/CryKeyProvider.h>
#include <boost/filesystem/path.hpp>
#include <cpp-utils/tempfile/TempFile.h>
#include <cpp-utils/io/Console.h>
#include <cpp-utils/random/RandomGenerator.h>
#include <cpp-utils/network/HttpClient.h>
#include <cpp-utils/SizedData.h>
#include <cryfs/impl/filesystem/CryDevice.h>
#include "CallAfterTimeout.h"
#include <cryfs/impl/config/CryConfigLoader.h>
#include <cryfs/impl/ErrorCodes.h>

namespace cryfs_cli {
    class Cli final {
    public:
        struct Credentials {
            boost::optional<string> password;
            SizedData givenHash;
            SizedData* returnedHash;
        };
        Cli(cpputils::RandomGenerator &keyGenerator, const cpputils::SCryptSettings& scryptSettings);
        fspp::fuse::Fuse* initFilesystem(const program_options::ProgramOptions &options, Credentials credentials);

    private:
        cryfs::CryConfigLoader::ConfigLoadResult _loadOrCreateConfig(const program_options::ProgramOptions &options, const cryfs::LocalStateDir& localStateDir, Credentials credentials);
        void _checkConfigIntegrity(const boost::filesystem::path& basedir, const cryfs::LocalStateDir& localStateDir, const cryfs::CryConfigFile& config, bool allowReplacedFilesystem);
        cpputils::unique_ref<cryfs::CryKeyProvider> _createKeyProvider(Credentials credentials);
        cpputils::either<cryfs::CryConfigFile::LoadError, cryfs::CryConfigLoader::ConfigLoadResult> _loadOrCreateConfigFile(boost::filesystem::path configFilePath, cryfs::LocalStateDir localStateDir, Credentials credentials, const boost::optional<std::string> &cipher, const boost::optional<uint32_t> &blocksizeBytes, bool allowFilesystemUpgrade, const boost::optional<bool> &missingBlockIsIntegrityViolation, bool allowReplacedFilesystem);
        boost::filesystem::path _determineConfigFile(const program_options::ProgramOptions &options);
        void _sanityChecks(const program_options::ProgramOptions &options);
        void _checkDirAccessible(const boost::filesystem::path &dir, const std::string &name, bool createMissingDir, cryfs::ErrorCode errorCode);
        void _sanityCheckFilesystem(cryfs::CryDevice *device);


        cpputils::RandomGenerator &_keyGenerator;
        cpputils::SCryptSettings _scryptSettings;
        boost::optional<cpputils::unique_ref<CallAfterTimeout>> _idleUnmounter;
        boost::optional<cpputils::unique_ref<cryfs::CryDevice>> _device;

        DISALLOW_COPY_AND_ASSIGN(Cli);
    };
}

#endif
