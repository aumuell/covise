/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef CONTROLLER_HOST_H
#define CONTROLLER_HOST_H

#include <thread>
#include <vector>
#include <map>
#include <mutex>

#include <comsg/CRB_EXEC.h>
#include <vrb/RemoteClient.h>
#include <vrb/client/VRBClient.h>
#include <comsg/coviseLaunchOptions.h>

#include "subProcess.h"
#include "config.h"

namespace covise
{
struct NEW_UI_HandlePartners;

namespace controller{




class HostManager;
struct UIOptions;
struct ModuleInfo;
class Userinterface;

struct RemoteHost : vrb::RemoteClient
{
    RemoteHost(const HostManager& manager, vrb::Program type, const std::string& sessionName = ""); //constructs a client with local information
    RemoteHost(const HostManager& manager, vrb::RemoteClient &&base); //constructs real remote client
    RemoteHost(RemoteHost &&other) = default;
    RemoteHost(const RemoteHost &other) = delete;
    
    const HostManager &hostManager;
    
    bool handlePartnerAction(covise::LaunchStyle action);
    covise::LaunchStyle state() const;
    void setTimeout(int seconds);
    bool startCrb(ShmMode shmMode);
    bool startUI(const UIOptions &options);
    const SubProcess &getProcess(sender_type type) const;
    SubProcess &getProcess(sender_type type);
    
    const NetModule &getModule(const std::string &name, int instance) const;
    NetModule &getModule(const std::string&name, int instance);

    void removeModule(NetModule &app, int alreadyDead);
    typedef std::vector<std::unique_ptr<SubProcess>> ProcessList;

    ProcessList::const_iterator begin() const;
    ProcessList::iterator begin();

    ProcessList::const_iterator end() const;
    ProcessList::iterator end();
    void launchProcess(const CRB_EXEC& exec) const;
    bool isModuleAvailable(const std::string &moduleName) const;
    NetModule &startApplicationModule(const string &name, const string &instanz, 
                          int posx, int posy, int copy, ExecFlag flags, NetModule *mirror = nullptr);
    bool get_mark() const;
    void reset_mark();
    void mark_save();
    bool removePartner();

private:
    bool addPartner();
    void launchScipt(vrb::Program exec, const std::vector<std::string> &cmdArgs);  //need to create a new remote host for these
    void launchManual(vrb::Program exec, const std::vector<std::string> &cmdArgs); //

    bool startUI(std::unique_ptr<Userinterface> &&ui, const UIOptions &options);
    void determineAvailableModules(const CRBModule &crb);
    void clearProcesses(); //order: modules->ui->crb

    ExecType m_exectype = ExecType::VRB;
    ProcessList m_processes;
    std::vector<const ModuleInfo*> m_availableModules; //contains references to hostmanagers available modules
    int m_shmID;
    int m_timeout = 30;
    bool m_saveInfo = false;

protected:
    covise::LaunchStyle m_state = covise::LaunchStyle::Disconnect;
    virtual void connectShm(const CRBModule &crbModule);

    virtual void launchCrb(vrb::Program exec, const std::vector<std::string> &cmdArgs);
};

struct LocalHost : RemoteHost{
    LocalHost(const HostManager& manager, vrb::Program type, const std::string& sessionName = "");
    virtual void connectShm(const CRBModule &crbModule) override;
    virtual void launchCrb(vrb::Program exec, const std::vector<std::string> &cmdArgs) override;
};
class HostManager
{
public:
    typedef std::map<int, std::unique_ptr<RemoteHost>> HostMap;
    HostManager();
    ~HostManager();
    struct UiState{
        int masterRequestSenderId = 0;
    } uiState;

    void sendPartnerList();
    std::vector<bool> handleAction(const covise::NEW_UI_HandlePartners &msg);
    void setOnConnectCallBack(std::function<void(void)> cb);
    int vrbClientID() const;
    const vrb::VRBClient &getVrbClient() const;
    
    LocalHost &getLocalHost();
    const LocalHost &getLocalHost() const;
    
    RemoteHost *getHost(int clientID);
    const RemoteHost *getHost(int clientID) const;

    RemoteHost &findHost(const std::string &ipAddress); //this will cause collaborative work on a single host to fail
    const RemoteHost &findHost(const std::string &ipAddress) const;

    std::vector<const SubProcess *> getAllModules(sender_type type) const;
    template<typename T>
    std::vector<const T*> getAllModules() const{
            auto v =  const_cast<HostManager *>(this)->getAllModules<T>();
            std::vector<const T *> modules(v.size());
            std::transform(v.begin(), v.end(), modules.begin(), [](T *t) {
                return t;
            });
           return modules;
    }
    template<typename T>
    std::vector<T*> getAllModules(){
        std::vector<T *> modules;
        for (auto &host : m_hosts)
        {
            for (auto &module : *host.second)
            {
                if (auto tmpMod = dynamic_cast<T *>(module.get()))
                {
                    modules.push_back(tmpMod);
                }
            }
        }
        return modules;
    }
    template<typename T>
    void sendAll(const Message &msg) const{
        for(const auto module : getAllModules<T>())
            module->send(&msg);
    }
    
    HostMap::const_iterator begin() const;
    HostMap::iterator begin();
    HostMap::const_iterator end() const;
    HostMap::iterator end();

    SubProcess *findModule(int peerID);
    bool slaveUpdate();
    mutable bool m_slaveUpdate = false;
    const Userinterface &getMasterUi() const;
    Userinterface &getMasterUi();
    std::string getHostsInfo() const; //get info string with all hosts
    const ModuleInfo &registerModuleInfo(const std::string &name, const std::string &category) const;
    void resetModuleInstances();

private:
    mutable std::set<ModuleInfo> m_availableModules; //every module that is available on at leaset one host. This manages the instance ids of the modules.
    std::unique_ptr<vrb::VRBClient> m_vrb;
    covise::ConnectionList m_connList;
    HostMap m_hosts;
    HostMap::iterator m_localHost;
    std::thread m_thread;
    mutable std::mutex m_mutex;
    std::function<void(void)> m_onConnectCallBack;
    std::atomic_int m_moduleCount{0};
    std::atomic_bool m_terminateVrb{false};
    covise::ServerConnection *m_crbConn = nullptr;
    void handleVrb();
    bool handleVrbMessage();

};
} //namespace controller
} //namespace covise
#endif