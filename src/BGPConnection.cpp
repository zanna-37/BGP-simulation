#include "BGPConnection.h"




BGPConnection :: BGPConnection(){

    stateMachine = new BGPStateMachine(this);

}
BGPConnection :: ~BGPConnection(){

    delete stateMachine;
}

// We should not need it
bool BGPConnection :: HandleEvent(Event event){

    return stateMachine->HandleEvent(event);

}

    



