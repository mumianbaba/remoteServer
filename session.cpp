#include <iostream>
#include <string.h>


#include "session.h"
#include "locallog.h"

using namespace std;


int Session::m_sidCount = 1;

Session::Session()
{
    m_sid = m_sidCount++;
    m_state = SStateRaw;
	m_inactive = 0;
    m_timeouts = 50;

    m_meetee[0].bev = NULL;
    m_meetee[0].userData = NULL;
    
    m_meetee[1].bev = NULL;
    m_meetee[1].userData = NULL;
}

Session::~Session()
{
    m_sid = 0;
    m_state = SStateDie;
    if (m_meetee[0].bev){
        bufferevent_free(m_meetee[0].bev);
        m_meetee[0].bev = NULL;
    }

    if (m_meetee[0].userData){
        free (m_meetee[0].userData);
        m_meetee[0].userData = NULL;
    }

    m_meetee[1].bev = NULL;

    if ( m_meetee[1].userData){
        free (m_meetee[1].userData);
        m_meetee[1].userData = NULL;
    }
}

int Session::setBev(int index, struct bufferevent * bev){

    if(!bev){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: setBev bev null");
        return -1;
    }

    if (index < 0 || index >= sizeof(m_meetee)/sizeof(Meetee)){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: setBev index error");
        return -1;
    }

    m_meetee[index].bev = bev;
    return 0;
}

int Session::setUserData(int index, void* userData, int len){

    if(!userData || len <= 0){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: setUserData userData null");
        return -1;
    }
    
    if (index < 0 || index >= sizeof(m_meetee)/sizeof(Meetee)){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: setUserData index error");
        return -1;
    }

    if(m_meetee[index].userData)
    {
        free (m_meetee[index].userData);
        m_meetee[index].userData = NULL;
    }

    m_meetee[index].userData = calloc(1, len);
    if(!m_meetee[index].userData){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: setUserData calloc error");
        return -1;
    }
	
	memcpy (m_meetee[index].userData, userData, len);
    return 0;
}

struct bufferevent * Session::getBev(int index)
{
    if (index < 0 || index >= sizeof(m_meetee)/sizeof(Meetee)){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: getBev index error");
        return NULL;
    }
    return  m_meetee[index].bev;
}

int Session::getUserData(int index, void* userData, int len){

	if (!userData || len <= 0){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: getUserData param error");
		return -1;
	}

    if (index < 0 || index >= sizeof(m_meetee)/sizeof(Meetee)){
        LocalLog::userLog(LOG_LEVEL_ERR, "error: getUserData index error");
        return -1;
    }

    if (!m_meetee[index].userData){
        cout<<"no user data"<<endl;
        return -1;
    }

	memcpy(userData, m_meetee[index].userData, len);
    return 0;
}

SessionState Session::getSessionState(){
    return m_state;
}

void Session::setSessionState(SessionState state){
    m_state =  state;
}

int Session::inactive(int interval){
   m_inactive+=interval;
   return (m_inactive >= m_timeouts)?1:0;
}

void Session::clearTimeout(){
   m_inactive = 0;
   return;
}

int Session::login(const char* data, int len)
{
    if (!data || len <= 0){
        LocalLog::userLog(LOG_LEVEL_ERR, "error:login data len null");
        return -1;
    }

    int ret = 0;

    if (m_state == SStateRaw){

        if (len < sizeof(TelProxyLogin)){
            LocalLog::userLog(LOG_LEVEL_ERR, "error:login  len less than login package");
            return -1;
        }

        char* sync = strnstr(data, len, TELPROXY_SYN, strlen(TELPROXY_SYN));
        if(!sync){
            printf ("login package no find the sync\n");
            return -1;
        }

        if ((len - (sync-data)) < sizeof(TelProxyLogin)){
            printf ("login package is sync is ok, but size is less than Struct\n");
            return -1;	
        }

        TelProxyLogin package;

        memcpy(&package, sync, sizeof(TelProxyLogin));

        package.type = ntohl(package.type);
        package.len =  ntohl(package.len);
        if (package.type != 1 && package.type != 2){
            printf ("login package type is not 1 or 2\n");
            return -1;
        }

        package.id[sizeof(package.id)-1] = '\0';
        if (package.id[0] == '\0'){
            printf ("login package id is null\n");
            return -1;
        }

        /* set user data for identify what type and who are you*/
        UCard ucard ;
        memset(&ucard, 0, sizeof(ucard));
        ucard.type = package.type;
        strncpy(ucard.id, package.id, sizeof(ucard.id));

        ret = setUserData(0, (void*)&ucard, sizeof(ucard));
        if (ret != 0){
            return -1;
        }

        printf ("welcome the Telnet Proxy, your type:%s, your id:%s\n",
                    (package.type == 1)?"cpe device" : "remote client",
                    package.id);

        m_state = SStateLogin;
        cout<<"login"<<endl;
        m_inactive = 0;
        /* login successful, set timeout 360s */
        m_timeouts = 6*60;

        /* write log session info */
        dump_info();
        return 0;
    }
    return -1;
}

int Session::logout(){

    if (m_state == SStateLogin || m_state== SStateBind){
        /* logout */
        m_state = SStateDie;
        cout<<"logout"<<endl;
        /* bind successful, set timeout 360s */
        m_inactive = 0;
        m_timeouts = 50;
        return 0;
    }
    return -1;
}

int Session::match_done(struct bufferevent * bev, UCard* card){
    if (!bev || !card){
        LocalLog::userLog(LOG_LEVEL_ERR, "error:match done param null");
        return -1;
    }
    int ret = 0;
    m_state = SStateBind;
    m_timeouts = 30*60;
    m_inactive = 0;
    ret = setBev(1, bev);
    if (ret != 0)
    {
        LocalLog::userLog(LOG_LEVEL_ERR, "error:match done set Bev error");
        return -1;
    }
    ret = setUserData(1, card, sizeof(UCard));
    if (ret != 0)
    {
        LocalLog::userLog(LOG_LEVEL_ERR, "error:match done set User Data error");
        return -1;
    }
    return 0;
}

int Session::match(Session* se){

    if (!se){
        LocalLog::userLog(LOG_LEVEL_ERR, "error:match session null");
        return -1;
    }

    int ret = 0;

    if(m_state == SStateLogin && 
       se->getSessionState() == SStateLogin){

        UCard* myCard = (UCard*)m_meetee[0].userData;
        UCard uCard;
        ret = se->getUserData(0, &uCard, sizeof(uCard));
        if (ret != 0){
            LocalLog::userLog(LOG_LEVEL_ERR, "error:match get u card error");
            return -1;
        }

        if((myCard->type == 1 &&  uCard.type != 2) || 
           (myCard->type == 2 &&  uCard.type != 1)){
            /* only type 1 match 2 or 2 match 1 */
            return -1;
        }

        if (strncmp(myCard->id, uCard.id, sizeof(myCard->id))){
            /*only match the same id */
            return -1;
        }

        /* save the peer bev and card, change the state */
        match_done(se->getBev(0), &uCard);
        se->match_done(getBev(0), myCard);
        cout<<"bind successful"<<endl;
#if 0
        /* match successful */
        setBev(1,  se->getBev(0));
        setUserData(1, &uCard, sizeof(UCard));
        m_state = SStateBind;

        se->setBev(1, getBev(0)); 
        se->setUserData(1, myCard, sizeof(UCard));
        se->setSessionState(SStateBind);
      
        cout<<"bind successful"<<endl;
        /* bind successful, set timeout 360s */
        m_timeouts = 30*60;
        m_inactive = 0;
#endif
        return 0;
    }
    return -1;
}

int Session::business(const char* data, int len)
{
    int ret = 0;
    struct bufferevent *bev = NULL;

    if (!data || len <= 0){
        LocalLog::userLog(LOG_LEVEL_ERR, "error:business data len null");
        return -1;
    }

    if(m_state == SStateBind)
    {
        //cout<<"do business"<<endl;

        /* clear the inactive count */
        m_inactive = 0;

        bev = m_meetee[1].bev;
        if (!bev){
            LocalLog::userLog(LOG_LEVEL_ERR, "error:business bev null");
            return -1;
        }

        ret = bufferevent_write(bev, data, len);
        if (ret != 0) {
            LocalLog::userLog(LOG_LEVEL_ERR, "error:business  send data failed");
            ret = -1;
        }
        return ret;
    }
    return -1;
}

void  Session::dump_info(void){
    int ret = 0;
    UCard* myCard = (UCard*)m_meetee[0].userData;
    const char* type = "none";
    const char* devid = "none";
    const char* state = "none";
    if (myCard){
        type = (myCard->type == 1)?"cpe device" : "remote client";
        devid = myCard->id;
    }
    if (m_state == SStateRaw){
        state = "raw";
    }else if (m_state == SStateLogin){
        state = "Login";
    }else if (m_state == SStateBind){
        state = "bind";
    } else if (m_state == SStateDie){
        state = "dei";
    }
    LocalLog::userLog(LOG_LEVEL_ALL, "session info:[\n\
                        type: %s\n\
                        id: %s\n\
                        state: %s\n\
                        sid: %d\n\
                        timeout: %d s\n\
                        ]\n", type, devid, state, m_sid, m_timeouts);
    return;
}