from typing import Optional
from pydantic import BaseModel
from fastapi import FastAPI, Response, Request, status
from fastapi import responses
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy import create_engine, Column, String, Integer, JSON, or_
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
import random
import string
from datetime import datetime, timedelta
import time
import uvicorn

#for multiprocesing
import threading


MOTD = "Welcome to the site"
SESSION_TIMEOUT_PERIOD_MINUTES = 30
SESSION_COLLECTOR_PERIOD_SECONDS = 60
SESSION_ID_SIZE_SYMBOLS = 5
#SQLALCHAMY_DATABASE_URL = 'mysql://remote_user:User_1234@192.168.0.105:3306/fastapi_test'
SQLALCHAMY_DATABASE_URL = 'sqlite:///site.db'

class session():

    def isValid(self):
        if datetime.utcnow() >= self.expire_time:
            return False
        else:
            return True

    def __init__(self, sess_id, username, expire_time:datetime):
        self.sess_id = sess_id
        self.expire_time = expire_time
        self.username = username

class sessionManager():

    def _sessionCollectorThread(self, timeout):
        while(self.runCollector):
            self.deleteInvalidSessions()
            time.sleep(timeout)

    def sessionCollectorStart(self, timeout):
        if self.runCollector is not True:
            self.runCollector = True
            self.collector = threading.Thread(target = self._sessionCollectorThread, daemon = True, args = (timeout,))
            self.collector.start()
    
    def sessionCollectorStop(self):
        self.runCollector = False

    def deleteInvalidSessions(self):
        for session in list(self.sessions):
            if session.isValid():
                pass
            else:
                self.sessions.remove(session)

    def getSessionByID(self, id):
        sess=None

        for session in self.sessions:
            if session.sess_id == id:
               if session.isValid():
                   sess = session
               else:
                   self.deleteInvalidSessions()
               break

        return sess 

    def addSession(self, session:session):
        self.sessions.append(session)

    def removeSessionById(self, id):
        for session in list(self.sessions):
            if session.sess_id == id:
                self.sessions.remove(session)
            else:
                pass    

    def __init__(self):
        self.sessions = []
        self.runCollector = False

def sessIdGenerator(N):
    return ''.join(random.SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(N))

class Credentials(BaseModel):
    username:str
    password:str

class UpdateSetting(BaseModel):
    setting_id:int
    value:dict

Base = declarative_base()
class User(Base):
    __tablename__ = "users"
    username = Column(String, primary_key = True)
    password = Column(String)

    def __init__(self, username, password):
        self.username = username
        self.password = password

class Setting(Base):
    __tablename__ = "settings"
    setting_id = Column(Integer, primary_key = True, autoincrement = True)
    device_name = Column(String)
    setting_name = Column(String)
    setting_type = Column(String)
    value = Column(JSON)

    def toDict(self):
        x = {
            "setting_id" : self.setting_id,
            "device_name" : self.device_name,
            "setting_name" : self.setting_name,
            "setting_type" : self.setting_type,
            "value" : self.value
        }
        return x

    def __init__(self, device_name, setting_name, setting_type, value):
        self.device_name = device_name
        self.setting_name = setting_name
        self.setting_type = setting_type
        self.value = value

class SettingAccess(Base):
    __tablename__ = "settings_access"
    id = Column(Integer, primary_key = True, autoincrement = True)
    setting_id = Column(Integer)
    username = Column(String)

    def __init__(self, setting_id, username):
        self.setting_id = setting_id
        self.username = username

class Device():

    def toDict(self):
        
        _settings = self.settings
        for i,s in enumerate(self.settings):
            _settings[i] = s.toDict()

        x = {
            "device_name" : self.name,
            "settings" : _settings
        }

        return x

    def __init__(self, name:string, settings:list):
        self.name = name 
        self.settings = settings



sm = sessionManager()
#engine = create_engine(SQLALCHAMY_DATABASE_URL)
engine = create_engine(SQLALCHAMY_DATABASE_URL, connect_args={"check_same_thread":False})
sqlSession= sessionmaker(bind=engine, autocommit=False,)()

def getSettingsForUserSortedByDevice(sess_id):
    username = sm.getSessionByID(sess_id).username
    accessibleSettingIds = [sa.setting_id for sa in sqlSession.query(SettingAccess).filter(SettingAccess.username == username).all()]
    accessibleSettings = [s for s in sqlSession.query(Setting).filter(Setting.setting_id.in_(accessibleSettingIds)).all()]
    devices=[]
    
    def isInList(name):

        for i,d in enumerate(devices):
            if (d.name == name):
                return i
        return -1

    for s in accessibleSettings:
        i = isInList(s.device_name)
        if i >= 0:
            devices[i].settings.append(s)
        else:
            devices.append(Device(s.device_name, [s,]))
    
    devices = [d.toDict() for d in devices]
    return devices

def getAccessibleSettingsIds(sess_id):
    username = sm.getSessionByID(sess_id).username
    accessibleSettingIds = [sa.setting_id for sa in sqlSession.query(SettingAccess).filter(SettingAccess.username == username).all()]
    return accessibleSettingIds
    
def validateSettingUpdate(updatedSetting:UpdateSetting, s:Setting):
    setting_type = s.setting_type
    value = updatedSetting.value

    #INTEGER
    if (type(value["value"]) is int) and setting_type == "integer":
        return True
    #BOOLEAN
    elif (type(value["value"]) is bool) and setting_type == "boolean":
        return True
    else:
        return False

origins = [
    "http://127.0.0.1:3000",
    "http://192.168.1.201:3000"
]

app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/", tags = ["general"])
def root(response: Response):
    return {"motd": MOTD}

@app.post("/login", tags = ["authentication"])
def login(response:Response, credentials:Credentials):
    result = sqlSession.query(User).filter(User.username == credentials.username).first()
    if result:
        if result.password == credentials.password:
            sess_id=sessIdGenerator(SESSION_ID_SIZE_SYMBOLS)
            time = (datetime.utcnow() + timedelta(minutes = SESSION_TIMEOUT_PERIOD_MINUTES))
            response.set_cookie(key="SESS_ID", value=sess_id,expires = time.ctime(), samesite = "strict", secure = False)
            sess=session(sess_id, credentials.username, time)
            sm.addSession(sess)
            sm.sessionCollectorStart(SESSION_COLLECTOR_PERIOD_SECONDS)
            return {"status":"Successful login"}
        else:
            response.status_code = status.HTTP_401_UNAUTHORIZED
            return {"status":"Wrong credentials"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status":"Wrong credentials"}


@app.get("/checksession", tags = ["authentication"])
def checkSession(request:Request, response:Response):
    

    if "SESS_ID" in request.cookies:
        sess=sm.getSessionByID(request.cookies["SESS_ID"])
        if sess is not None:
            response.status_code = status.HTTP_200_OK
            return {"status" : "Valid", "username" : sess.username}
        else:
            response.status_code = status.HTTP_403_FORBIDDEN
            return {"status": "Invalid"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status": "Invalid"}

@app.post("/logout", tags = ["authentication"])
def logout(request:Request, response:Response):

    if "SESS_ID" in request.cookies:
        sess=sm.getSessionByID(request.cookies["SESS_ID"])
        if sess is not None:
            response.status_code = status.HTTP_200_OK
            sm.removeSessionById(sess.sess_id)
            return {"status" : "Logged out", "username" : sess.username}
        else:
            response.status_code = status.HTTP_404_NOT_FOUND
            return {"status": "Session not found"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status": "No token applied to request"}


@app.get("/user", tags = ["user content"])
def userpage(request:Request, response:Response):
    

    if "SESS_ID" in request.cookies:
        sess=sm.getSessionByID(request.cookies["SESS_ID"])
        if sess is not None:
            response.status_code = status.HTTP_200_OK
            return {"userdata" : "This is test user data"}
        else:
            response.status_code = status.HTTP_403_FORBIDDEN
            return {"status": "No session"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status": "No token applied to request"}

@app.get("/user/settings", tags = ["user content"])
def  userSettings(request:Request, response:Response):
    
    returnStatus = "No token applied to request"
    devices = []

    if "SESS_ID" in request.cookies:
        sess=sm.getSessionByID(request.cookies["SESS_ID"])
        if sess is not None:
            response.status_code = status.HTTP_200_OK
            returnStatus = "Success"
            devices = getSettingsForUserSortedByDevice(request.cookies["SESS_ID"])
        else:
            response.status_code = status.HTTP_403_FORBIDDEN
            return {"status": "No session"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status": "No token applied to request"}

    

    return {"status" : returnStatus, "devices" : devices}   


@app.put("/user/settings/{id:int}", tags = ["user content"])
def  updateSetting(updatedSetting:UpdateSetting, response:Response, request:Request, id):
    
    returnStatus = "No token applied to request"

    if "SESS_ID" in request.cookies:
        sess=sm.getSessionByID(request.cookies["SESS_ID"])
        if sess is not None:
            accessibleSettingsIds = getAccessibleSettingsIds(request.cookies["SESS_ID"])
            if id in accessibleSettingsIds:
                s = sqlSession.query(Setting).filter(Setting.setting_id == id).one()
                if s is not None:
                    if validateSettingUpdate(updatedSetting, s):
                        response.status_code = status.HTTP_200_OK
                        returnStatus = "Success"
                        s.value = updatedSetting.value
                        sqlSession.commit()
                    else:
                        response.status_code = status.HTTP_422_UNPROCESSABLE_ENTITY
                        returnStatus = "Invalid value type "
                else:
                    response.status_code = status.HTTP_404_NOT_FOUND
                    returnStatus = "No setting with that id"
            else:
                response.status_code = status.HTTP_403_FORBIDDEN
                returnStatus = "No user access to that setting"

        else:
            response.status_code = status.HTTP_403_FORBIDDEN
            return {"status": "No session"}
    else:
        response.status_code = status.HTTP_401_UNAUTHORIZED
        return {"status": "No token applied to request"}

    return {"status" : returnStatus}

      