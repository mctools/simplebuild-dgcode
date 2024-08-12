#include "G4Utils/STUserActionInitHelper.hh"
#include "G4VUserActionInitialization.hh"
#include "G4RunManager.hh"
#include <mutex>
#include <atomic>

namespace G4Utils {

  //Flexible helper class which can be used to create and register a custom
  //G4VUserActionInitialization but based on std::function's (i.e. making it
  //possible to use lambda functions, etc.).

  namespace UserActionInitHelper {

    namespace {
      class STUserActionInitHelper final : public G4VUserActionInitialization {
        mutable std::atomic<unsigned> m_nbuildcalled = {0};
        // G4UserSteppingAction* m_ua_stepping = nullptr;
        // G4UserEventAction* m_ua_event = nullptr;
        // G4VUserPrimaryGeneratorAction* m_ua_primgen = nullptr;
        mutable std::function<G4UserSteppingAction*()> m_fct_stepping;
        mutable std::function<G4UserEventAction*()> m_fct_event;
        mutable std::function<G4VUserPrimaryGeneratorAction*()> m_fct_primgen;
        mutable std::function<G4UserSteppingAction*(G4UserSteppingAction*)> m_wrapfct_stepping;
        mutable std::function<G4UserEventAction*(G4UserEventAction*)> m_wrapfct_event;
      public:

        void addWrappingUserSteppingActionFct( std::function<G4UserSteppingAction*(G4UserSteppingAction*)> f )
        {
          if ( m_nbuildcalled.load() > 0 )
            throw std::runtime_error("Adding (wrapping) UserSteppingAction too late.");
          m_wrapfct_stepping = std::move(f);
        }

        void addWrappingUserEventActionFct( std::function<G4UserEventAction*(G4UserEventAction*)> f )
        {
          if ( m_nbuildcalled.load() > 0 )
            throw std::runtime_error("Adding (wrapping) UserEventAction too late.");
          m_wrapfct_event = std::move(f);
        }

        void addUserSteppingActionFct( std::function<G4UserSteppingAction*()> f )
        {
          if ( m_nbuildcalled.load() > 0 )
            throw std::runtime_error("Adding UserSteppingAction too late.");
          m_fct_stepping = std::move(f);//NB: We allow overwriting??
        }

        void addUserEventActionFct( std::function<G4UserEventAction*()> f )
        {
          if ( m_nbuildcalled.load() > 0 )
            throw std::runtime_error("Adding UserEventAction too late.");
          m_fct_event = std::move(f);//NB: We allow overwriting??
        }

        void addUserPrimaryGeneratorActionFct( std::function<G4VUserPrimaryGeneratorAction*()> f )
        {
          if ( m_nbuildcalled.load() > 0 )
            throw std::runtime_error("Adding UserPrimaryGeneratorAction too late.");
          m_fct_primgen = std::move(f);//NB: We allow overwriting??
        }

        // G4UserSteppingAction* getUserSteppingAction()
        // {
        //   return m_ua_stepping;
        // }

        // G4UserEventAction* getUserEventAction()
        // {
        //   return m_ua_event;
        // }

        // G4VUserPrimaryGeneratorAction* getUserPrimaryGeneratorAction()
        // {
        //   return m_ua_primgen;
        // }

        // void addUserActionFactoryFct( std::function<AllActions()> f )
        // {
        //   if ( m_nbuildcalled.load() > 0 )
        //     throw std::runtime_error("Adding user action hook too late "
        //                              "(via addUserActionFactoryFct).");
        //   m_delayedfcts.emplace_back( std::move(f) );
        // }

        // void add( G4UserSteppingAction* act )
        // {
        //   if ( m_nbuildcalled.load() > 0 )
        //     throw std::runtime_error("Adding G4UserSteppingAction* too late.");
        //   //This is allowed! (so calling code can implement wrapping)
        //   //if ( m_ua_stepping )
        //   //  throw std::runtime_error("Already added G4UserSteppingAction*.");
        //   m_ua_stepping = act;
        // }

        // void add( G4UserEventAction* act )
        // {
        //   if ( m_nbuildcalled.load() > 0 )
        //     throw std::runtime_error("Adding G4UserEventAction* too late.");
        //   //This is allowed! (so calling code can implement wrapping)
        //   //if ( m_ua_event )
        //   //  throw std::runtime_error("Already added G4UserEventAction*.");
        //   m_ua_event = act;
        // }

        // void add( G4VUserPrimaryGeneratorAction* act )
        // {
        //   if ( m_nbuildcalled.load() > 0 )
        //     throw std::runtime_error("Adding G4VUserPrimaryGeneratorAction* too late.");
        //   //This is allowed! (so calling code can implement wrapping)
        //   //if ( m_ua_primgen )
        //   //  throw std::runtime_error("Already added G4VUserPrimaryGeneratorAction*.");
        //   m_ua_primgen = act;
        // }

        void Build() const override
        {
          //throw std::runtime_error("test");
          auto nbuildprev = m_nbuildcalled.fetch_add(1);
          if ( nbuildprev > 0 )
            throw std::runtime_error("MT-mode not supported in dgcode"
                                     " Launcher (::Build() called more than once).");



          // if ( auto& f : m_delayedfcts ) {
          //   auto a = f();
          //   if ( a.stepping ) {
          //     if ( m_ua_stepping )
          //       throw std::runtime_error("error multiple Stepping....");
          //     m_ua_stepping = a.stepping;
          //   }
          //   if ( a.event ) {
          //     if ( m_ua_event )
          //       throw std::runtime_error("error multiple Event....");
          //     m_ua_event = a.event;
          //   }
          //   if ( a.primgen ) {
          //     if ( m_ua_primgen )
          //       throw std::runtime_error("error multiple Primgen....");
          //     m_ua_primgen = a.primgen;
          //   }
          // }
          // m_delayedfcts.clear();

          if ( m_wrapfct_stepping ) {
            if (!m_fct_stepping)
              this->SetUserAction( m_wrapfct_stepping(nullptr) );
            else
              this->SetUserAction( m_wrapfct_stepping(m_fct_stepping()) );
          } else {
            if ( m_fct_stepping ) {
              this->SetUserAction( m_fct_stepping() );
              m_fct_stepping = nullptr;
            }
          }
          if ( m_wrapfct_event ) {
            if (!m_fct_event)
              this->SetUserAction( m_wrapfct_event(nullptr) );
            else
              this->SetUserAction( m_wrapfct_event(m_fct_event()) );
          } else {
            if ( m_fct_event ) {
              this->SetUserAction( m_fct_event() );
              m_fct_event = nullptr;
            }
          }

          if ( m_fct_primgen ) {
            this->SetUserAction( m_fct_primgen() );
            m_fct_primgen = nullptr;
          }
          //        this->SetUserAction( ... )
        }
        //   // Virtual method to be implemented by the user to instantiate user run action
        //   // class object to be used by G4MTRunManager. This method is not invoked in
        //   // the sequential mode. The user should not use this method to instantiate
        //   // user action classes except for user run action.
        void BuildForMaster() const override
        {
          //Do nothing!
          // throw std::runtime_error("MT-mode not supported in dgcode"
          //                          " Launcher (::BuildForMaster() called).");
        }

        //   // protected:
        //   //   // These methods should be used to define user's action classes.
        //   //   void SetUserAction(G4VUserPrimaryGeneratorAction*) const;
        //   //   void SetUserAction(G4UserRunAction*) const;
        //   //   void SetUserAction(G4UserEventAction*) const;
        //   //   void SetUserAction(G4UserStackingAction*) const;
        //   //   void SetUserAction(G4UserTrackingAction*) const;
        //   //   void SetUserAction(G4UserSteppingAction*) const;
      };

      struct UAIHelperDB {
        std::mutex mutex;
        STUserActionInitHelper * helper = nullptr;
        bool registeredWithRM = false;
        UAIHelperDB() : helper(new STUserActionInitHelper) {}
      };
      UAIHelperDB& getUAIHelperDB()
      {
        static UAIHelperDB db;
        return db;
      };
    }

    void ensureRegisterWithRunManager()
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      if ( db.registeredWithRM )
        return;
      G4RunManager * rm = G4RunManager::GetRunManager();
      if (!rm)
        throw std::runtime_error("RunManager not initialised");
      rm->SetUserInitialization( db.helper );
      db.registeredWithRM = true;
    }

    // void addUserSteppingAction( G4UserSteppingAction* act )
    // {
    //   if (!act)
    //     throw std::runtime_error("Adding null G4UserSteppingAction");
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   db.helper->add( act );
    // }

    // void addUserEventAction( G4UserEventAction* act )
    // {
    //   if (!act)
    //     throw std::runtime_error("Adding null G4UserEventAction");
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   db.helper->add( act );
    // }

    // void addUserPrimaryGeneratorAction( G4VUserPrimaryGeneratorAction* act )
    // {
    //   if (!act)
    //     throw std::runtime_error("Adding null G4VUserPrimaryGeneratorAction");
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   db.helper->add( act );
    // }

    // G4UserSteppingAction* getUserSteppingAction()
    // {
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   return db.helper->getUserSteppingAction();
    // }

    // G4UserEventAction* getUserEventAction()
    // {
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   return db.helper->getUserEventAction();
    // }

    // G4VUserPrimaryGeneratorAction* getUserPrimaryGeneratorAction()
    // {
    //   auto& db = getUAIHelperDB();
    //   std::lock_guard<std::mutex> lock(db.mutex);
    //   return db.helper->getUserPrimaryGeneratorAction();
    // }

    void addUserSteppingActionFct( std::function<G4UserSteppingAction*()> f )
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      db.helper->addUserSteppingActionFct( std::move(f) );
    }

    void addUserEventActionFct( std::function<G4UserEventAction*()> f )
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      db.helper->addUserEventActionFct( std::move(f) );
    }
    void addUserPrimaryGeneratorActionFct( std::function<G4VUserPrimaryGeneratorAction*()> f )
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      db.helper->addUserPrimaryGeneratorActionFct( std::move(f) );
    }

    void addWrappingUserSteppingActionFct( std::function<G4UserSteppingAction*(G4UserSteppingAction*)> f )
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      db.helper->addWrappingUserSteppingActionFct( std::move(f) );
    }

    void addWrappingUserEventActionFct( std::function<G4UserEventAction*(G4UserEventAction*)> f )
    {
      auto& db = getUAIHelperDB();
      std::lock_guard<std::mutex> lock(db.mutex);
      db.helper->addWrappingUserEventActionFct( std::move(f) );
    }


  }
}
