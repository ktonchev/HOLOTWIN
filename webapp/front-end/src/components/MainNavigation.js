import { Link } from 'react-router-dom';
import settings from '../settings/Settings.js';
import { useState, useEffect } from 'react';

import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import Button from '@mui/material/Button';
import HomeIcon from '@mui/icons-material/Home';
import LoginModal from './LoginModal';
import { makeStyles } from '@material-ui/core'

const drawerWidth = 240

const useStyles = makeStyles((theme) => {
  
  return {
    page: {
      background: '#f9f9f9',
      width: '100%',
      padding: theme.spacing(3),
    },
    root: {
      display: 'flex',
    },
    drawer: {
      width: drawerWidth,
    },
    drawerPaper: {
      width: drawerWidth,
    },
    active: {
      background: '#f4f4f4'
    },
    title: {
      padding: theme.spacing(2),
    },
    appBar: {
      width: `calc(100% - ${drawerWidth}px)`,
    },
    date: {
      flexGrow: 1
    },
    toolbar: theme.mixins.toolbar,
    avatar: {
      marginLeft: theme.spacing(2)
    }
  }
})

function MainNavigation() {
    
    const [modalOpen, setModalOpen] = useState(false);
    const [loggedIn, setLoggedIn] = useState(false);
    const classes = useStyles()

    let accountActionButton =  <Button color="secondary" variant = "contained" onClick = {onLoginClick}>Login</Button>


    async function checkSession(){
        let reachedServer = true;
        console.log(settings.backend_address);
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
            if(response.status === "Valid" && loggedIn === false){      
                setLoggedIn(true);
            }
            else if (response.status === "Invalid" && loggedIn === true){
                setLoggedIn(false);
            }
        }
    }

    useEffect(() => {
        const interval = setInterval(() => {
            if(loggedIn){
                checkSession();
            }
        }, settings.checkSession_period_ms);
      
        return () => clearInterval(interval); // This represents the unmount function, in which you need to clear your interval to prevent memory leaks.
    }, [loggedIn])

    function onLoginClick(){
        setModalOpen(true);
    }

    if (loggedIn === true) {
        accountActionButton = <Button color="secondary" variant = "contained" onClick = {onLoginClick}>Logout</Button>;
    }

    checkSession();

        return (
        <div className={classes.root}>
            <AppBar 
                position='relative' 
                className={classes.appBar}
                elevation={0}
                color="primary">
                <Toolbar>
                    <Box display='flex' flexGrow={1}>
                        <Button component={Link} to="/" color = "inherit" variant = "outlined" startIcon = {<HomeIcon />}>Home</Button>
                        <Button component={Link} to="/dashboard" color="inherit">Dashboard</Button>
                        <Button component={Link} to="/settings" color="inherit">Device Settings</Button>  
                    </Box>
                    {accountActionButton}
                </Toolbar>
            </AppBar>
            <LoginModal modalOpen={modalOpen} setModalOpen={setModalOpen}  loggedIn= {loggedIn} setLoggedIn = {setLoggedIn}/>
        </div>   
    );
}
  
export default MainNavigation;