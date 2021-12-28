import React from 'react';
import { Route, Switch } from 'react-router-dom';

import HomePage from './pages/HomePage';
import Dashboard from './pages/Dashboard';
import SettingsPage from './pages/SettingsPage';
import MainNavigation from './components/MainNavigation';

import { createTheme, ThemeProvider } from '@mui/material/styles';

import CssBaseline from '@mui/material/CssBaseline';
import { Home } from '@mui/icons-material';

const theme = createTheme({
  palette: {
    type: 'light',
    primary: {
      main: '#083D77',
    },
    secondary: {
      main: '#DA4167',
    },
    background: {
      paper: '#EBEBD3',
    },
  },  
  
  typography: {
    fontFamily: ['Roboto', 'sans-serif'].join(','),
  },

  // components: {
  //   MuiPaper: {
  //     styleOverrides: {
  //       root: {
  //         padding: '10px',
  //         marginBottom: '10px',
  //         backgroundColor: '#DA4167',
  //       }  
  //     },
  //   },
  // }
});


function App() {
  
  return (
        <ThemeProvider theme = {theme}>
          <CssBaseline />
            <MainNavigation />
              <Switch>
                <Route path='/' exact>
                  <HomePage />
                </Route>
                <Route path='/dashboard'>
                  <Dashboard />
                </Route>
                <Route path='/settings'>
                  <SettingsPage />
                </Route>
              </Switch>
        </ThemeProvider>
  );
}

export default App;
