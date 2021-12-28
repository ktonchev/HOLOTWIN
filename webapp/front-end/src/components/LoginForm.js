import { useHistory } from "react-router-dom";
import { useRef } from 'react';
import { useState } from 'react';
import settings from '../settings/Settings';
import React from 'react';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import { Input, InputLabel, FormHelperText } from "@mui/material";

function LoginForm (props){

    const history = useHistory();
    const usernameRef = useRef();
    const passwordRef = useRef();

    const [formState, setFormState] = useState(<Typography variant = "h6" sx={{fontWeight : "bold"}}>Please enter credentials</Typography>);

    function loginRequest(_credentials){
        let result = fetch(
            settings.backend_address + "login",
            {
              method: 'POST',
              mode: 'cors',
              credentials: 'include',
              headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
              },
              body: JSON.stringify(_credentials) 
            }
        );
        
        return result;
    }

    async function submitHandler(event){
        let reachedServer= true;
        event.preventDefault();
        const enteredUsername = usernameRef.current.value;
        const enterePassword = passwordRef.current.value;

        const credentials = {
            username : enteredUsername,
            password : enterePassword
        }

        let response = await loginRequest(credentials).catch(() => {alert("Could not connect to back-end server!");reachedServer=false});
        if(reachedServer){
            if (response.status === 200){
                setFormState(<Typography variant = "h6" sx={{fontWeight : "bold"}}>Logged in</Typography>);
                props.setModalOpen(false)
                props.setLoggedIn(true)
                moveToDashboard();
            }
            else{
                setFormState(<Typography variant = "h6" color = "error" sx={{fontWeight : "bold"}}>WRONG CREDENTIALS!</Typography>);
            }
        }
    }

    function moveToDashboard() {
        history.replace('/Dashboard');
        
    }

    return (

        <form onSubmit={submitHandler}>
            
            <FormHelperText id="my-helper-text">
                {formState}
            </FormHelperText>

            <div>
                <InputLabel htmlFor='username'>Username </InputLabel >
                <Input type='username' required id='username' inputRef={usernameRef}/>
            </div>
            <div>
                <InputLabel  htmlFor='password'>Password </InputLabel>
                <Input type='password' required id='password' inputRef={passwordRef}/>
            </div>
            

            <Button type = "submit" color="primary" >Login</Button>
            
        </form>
    );

}

export default LoginForm;