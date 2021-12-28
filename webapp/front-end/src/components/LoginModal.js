import * as React from 'react';
import Box from '@mui/material/Box';
import Modal from '@mui/material/Modal';
import LoginForm from './LoginForm';
import LogoutForm from './LogoutForm';

const style = {
  position: 'absolute',
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
  width: 400,
  bgcolor: 'background.paper',
  border: '2px solid #000',
  boxShadow: 24,
  p: 4,
};

function LoginModal(props) {
  const modalOpen = props.modalOpen;
  const setModalOpen = props.setModalOpen;
  const handleClose = () => setModalOpen(false);

  let form = <LoginForm setModalOpen = {setModalOpen} modalOpen = {modalOpen} logedIn = {props.loggedIn} setLoggedIn = {props.setLoggedIn}/>;

  if(props.loggedIn ===true){
    form =<LogoutForm setModalOpen = {setModalOpen} modalOpen = {modalOpen} logedIn = {props.loggedIn} setLoggedIn = {props.setLoggedIn}/>
  }

  return (
    <div>
      <Modal
        open={modalOpen}
        onClose={handleClose}
        aria-labelledby="modal-modal-title"
        aria-describedby="modal-modal-description"
      >
        <Box sx={style}>
          {form}
        </Box>
      </Modal>
    </div>
  );
}

export default LoginModal;