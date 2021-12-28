import { useHistory } from "react-router-dom";
import settings from '../settings/Settings.js';
import { useState } from 'react';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

function LogoutForm(props) {

    const history = useHistory();
    const [loginStatus, setLoginStatus] = useState("Invalid");

    if (typeof LogoutForm.username == 'undefined'){
        LogoutForm.username = "";
    }
    let display;

    function moveToStartPage() {
        props.setModalOpen(false);
        history.replace('/');
    }

    async function logoutRequest(){
        let reachedServer = true;
        let request = await fetch(
            settings.backend_address + "logout",
            {
              method: 'POST',
              mode: 'cors',
              credentials: 'include',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
              } 
            }
        ).catch(() => {alert("Could not connect to back-end server!");reachedServer=false});
        if(reachedServer){
            let response = await request.json();
            return response;
        }
        else{
            return null; // ???? Taka li trqbva da e ????
        }
    }

    async function checkSession(){
        let reachedServer = true;
        const request = await fetch(
          settings.backend_address + "checksession",
          {
            method: 'GET',
            mode: 'cors',
            credentials: 'include'
          }
        ).catch(() => {alert("Could not connect to back-end server!");reachedServer=false});
        if(reachedServer){
            let response = await request.json();
            if(loginStatus !== response.status){      
                if(response.status === "Valid"){
                    LogoutForm.username = response.username;
                }
                else{
                    LogoutForm.username = "";
                }
                setLoginStatus(response.status);
            }
        }
    }

    async function onLogoutClick(){
        if(loginStatus==="Valid"){
            let response = await logoutRequest();
            let logoutStatus = response.status;
            let newLoginStatus = loginStatus;
            if(logoutStatus === "Logged out"){
                document.cookie = "SESS_ID= ; expires = Thu, 01 Jan 1970 00:00:00 GMT";
                newLoginStatus = "Invalid";
            }
            if(loginStatus !== newLoginStatus){
                setLoginStatus(newLoginStatus);
            }
        }
        props.setModalOpen(false);
        props.setLoggedIn(false)
        history.replace('/');
    }
    
    checkSession();
    if(loginStatus==="Valid"){
        display = <>
            <Typography variant = "h6" sx={{fontWeight : "bold"}}>Logged in as {LogoutForm.username}</Typography>
            <Button color="primary" onClick= {onLogoutClick}>Logout</Button>
        </>;
    }
    else{
        display = <>
            <Typography variant = "h6" sx={{fontWeight : "bold"}}>You are not logged in!</Typography>
            <Button color="primary" onClick= {moveToStartPage}>To start page ...</Button>
        </>;
    }

    return (
        <div>
            {display}
        </div>
    );
  }
  
export default LogoutForm;