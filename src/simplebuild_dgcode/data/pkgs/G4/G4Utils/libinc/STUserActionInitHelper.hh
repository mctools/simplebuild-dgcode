#ifndef G4Utils_STUserActionInitHelper_hh
#define G4Utils_STUserActionInitHelper_hh

//#include "G4VUserActionInitialization.hh"
#include <functional>

class G4UserSteppingAction;
class G4UserEventAction;
class G4VUserPrimaryGeneratorAction;

namespace G4Utils {

  //Flexible helper class which can be used to create and register a custom
  //G4VUserActionInitialization but based on std::function's (i.e. making it
  //possible to use lambda functions, etc.).

  namespace UserActionInitHelper {

    void addUserSteppingActionFct( std::function<G4UserSteppingAction*()> );
    void addUserEventActionFct( std::function<G4UserEventAction*()> );
    void addUserPrimaryGeneratorActionFct( std::function<G4VUserPrimaryGeneratorAction*()> );

    void addWrappingUserSteppingActionFct( std::function<G4UserSteppingAction*(G4UserSteppingAction*)> );
    void addWrappingUserEventActionFct( std::function<G4UserEventAction*(G4UserEventAction*)> );

    // void addUserSteppingAction( G4UserSteppingAction* );
    // void addUserEventAction( G4UserEventAction* );
    // void addUserPrimaryGeneratorAction( G4VUserPrimaryGeneratorAction* );

    // G4UserSteppingAction* getUserSteppingAction();//nullptr if not added
    // G4UserEventAction* getUserEventAction();//nullptr if not added
    // G4VUserPrimaryGeneratorAction* getUserPrimaryGeneratorAction();//nullptr if not added

    //FIXME: also G4UserStackingAction
    //FIXME: also G4UserTrackingAction
    void ensureRegisterWithRunManager();

  }

}
  // private:
  //   STUserActionInitHelper() = default;
  //   static STUserActionInitHelper * instance() {
  //     static std::atomic<bool> already_registered_init = false;

  //   };

  //   G4UserSteppingAction * m_ua_stepping = nullptr;
  //   G4UserEventAction * m_ua_event = nullptr;
  //   std::atomic<unsigned> m_nbuildcalled = {0};
  // };


  // class STUserActionInitHelper final : public G4VUserActionInitialization {
  // public:

  //   using fct_build_usa_t = std::function<G4UserSteppingAction*()>;
  //   using fct_build_uea_t = std::function<G4UserEventAction*()>;
  //   fct_build_usa_t m_usa;
  //   fct_build_uea_t m_uea;
  //   bool m_already_called_build = false;
  // public:

  //   fct_build_usa_t stealUserSteppingAction()
  //   {
  //     fct_build_usa_t f = std::move(m_usa);
  //     m_usa = nullptr;
  //     return std::move(f);
  //   }

  //   fct_build_uea_t stealUserEventAction()
  //   {
  //     fct_build_uea_t f = std::move(m_uea);
  //     m_uea = nullptr;
  //     return std::move(f);
  //   }

  //   void addUserSteppingAction( G4UserSteppingAction* usa )
  //   {
  //     addUserSteppingAction([usa](){ return usa; });
  //   }

  //   void addUserEventAction( G4UserEventAction* uea )
  //   {
  //     addUserEventAction([uea](){ return uea; });
  //   }

  //   void addUserSteppingAction( fct_build_usa usa )
  //   {
  //     if ( m_usa!=nullptr )
  //       throw std::runtime_error("STUserActionInitHelper::addUserSteppingAction already called");
  //     m_usa = std::move(usa);
  //   }

  //   void addUserEventAction( fct_build_uea uea )
  //   {
  //     if ( m_uea!=nullptr )
  //       throw std::runtime_error("STUserActionInitHelper::addUserEventAction already called");
  //     m_uea = std::move(uea);
  //   }

  //   void Build() const override
  //   {
  //     if (m_already_called_build)
  //       throw std::runtime_error("MT-mode not supported in dgcode"
  //                                " Launcher (::Build() called more than once).");
  //     m_already_called_build = true;

  //     if ( m_usa )
  //       this->SetUserAction(m_usa());
  //     m_usa = nullptr;

  //     if ( m_uea )
  //       this->SetUserAction(m_uea());
  //     m_uea = nullptr;
  //   }

  //   // Virtual method to be implemented by the user to instantiate user run action
  //   // class object to be used by G4MTRunManager. This method is not invoked in
  //   // the sequential mode. The user should not use this method to instantiate
  //   // user action classes except for user run action.
  //   void BuildForMaster() const override
  //   {
  //     throw std::runtime_error("MT-mode not supported in dgcode"
  //                              " Launcher (::BuildForMaster() called).");
  //   }

  //   // protected:
  //   //   // These methods should be used to define user's action classes.
  //   //   void SetUserAction(G4VUserPrimaryGeneratorAction*) const;
  //   //   void SetUserAction(G4UserRunAction*) const;
  //   //   void SetUserAction(G4UserEventAction*) const;
  //   //   void SetUserAction(G4UserStackingAction*) const;
  //   //   void SetUserAction(G4UserTrackingAction*) const;
  //   //   void SetUserAction(G4UserSteppingAction*) const;
  // };
//}

#endif
