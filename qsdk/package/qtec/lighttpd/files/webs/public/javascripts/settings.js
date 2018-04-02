$('.switch-icon').click(function settingSwitch() {
    $(this).prev().attr('checked',!$(this).prev().is(':checked'));
})
resize();
window.onresize=function () {
    resize()
}
function resize() {
    $('.nav-list').removeClass('hide')
    var w=document.body.clientWidth;
    // var nw=$('.nav-list')[0].offsetWidth;
    var nl=(w-1160)/2-222;
    if(w<1603){
        $('.nav-list').addClass('row-list');
        $('.nav-list').removeAttr('style');
        $('.container').css('padding-top','106px');
    }else {
        $('.nav-list').attr('style','left:'+nl+'px')
        $('.nav-list').removeClass('row-list');
        $('.nav-list li').removeAttr('style');
        $('.container').removeAttr('style');
    }
}


function get_html_name(m) {
    switch (m){
        case '#wifi-setting':
            return 'wifiSettings';
            break;
        case '#connect-ways':
            return 'connectWays';
            break;
        case '#manage-pwd-modify':
            return 'managePwdModify';
            break;
        default:
            return 'wifiSetting';
            break;
    }
}

$('.nav-list ul li').click(function () {
    var oldindex=$('.nav-list ul li a.active').parent().index();
    if(!$(this).children().hasClass('active')){
        $('.nav-list ul li a.active').removeClass('active');
        $(this).children().addClass('active');
    }
    // var html_name=get_html_name($(this).children().attr('href'));
    // getHtml(html_name)
    var i=$(this).index();
    if(oldindex!=i){
        // pagereset(i);
    }
    // pagereset(oldindex,i);
    if(!$('.frame-list>div').eq(i).hasClass('show')){
        $('.frame-list>div.show').hide();
        $('.frame-list>div.show').removeClass('show');
        $('.frame-list>div').eq(i).addClass('show');
        $('.frame-list>div').eq(i).fadeIn();
    }
    if($('.error-info').html().length>0){
        errorhide();
    }
    // oldindex=i;
})
$('.nav-list ul li').click(function (){$('html,body').animate({scrollTop: '0px'}, 800);})

function pagereset(m,n) {
    if(m!=n){
        switch (m){
            case 0:
                wifiInfo(2);
                break;
            case 1:
                getconnectways();
                $('.pppoe-pwd input').attr('type','password');
                $('.pppoe-pwd .pwd-switch').attr('class','pwd-switch hidePassword');
                break;
            case 2:
                $('.manage-pwd-modify input').val('');
                $('.manage-pwd-modify input').attr('type','password');
                $('.manage-pwd-modify .pwd-switch').attr('class','pwd-switch hidePassword');
                break;
            case 3:
                gettask();
                break;
            case 4:
                getlanInfo();
                break;
            default:
                break;
        }
    }
}

$('.normal-switch').click(function () {
    if($('.normal-switch-input').is(':Checked')){
        $('.normal-settings .settings-form input').removeAttr('disabled');
        $('.normal-settings .settings-form select').removeAttr('disabled');
    }else {
        $('.normal-settings .settings-form input').attr('disabled','disabled');
        $('.normal-settings .settings-form select').attr('disabled','disabled');
    }
    wifiInfo(0);
})
$('.speed-switch').click(function () {
    if($('.speed-switch-input').is(':Checked')) {
        $('.speed-settings .settings-form input').removeAttr('disabled');
        $('.speed-settings .settings-form select').removeAttr('disabled');
    }else {
        $('.speed-settings .settings-form input').attr('disabled', 'disabled');
        $('.speed-settings .settings-form select').attr('disabled', 'disabled');
    }
    wifiInfo(1);
})

$('.wifi-setting-icon').parent().click(function () {
    wifiInfo(2);
})
function wifiInfo(e) {
    var data={
        "requestUrl": "get_wireless_cfg",
        "method": "get"
    };
    if(e==0){
        postAjax(data,normalwifiupdate)
    }
    if(e==1){
        postAjax(data,speedwifiupdate);
    }
    if(e==2) {
        postAjax(data, wifiinfoset)
    }
}
function wifiinfoset(res) {
    normalwifiupdate(res);
    if(res.data.disabled1=='0'){
        $('.normal-switch-input').attr('checked',true);
        $('.normal-settings .settings-form input').removeAttr('disabled');
    }else {
        $('.normal-switch-input').removeAttr('checked');
        $('.normal-settings .settings-form input').attr('disabled','disabled');
    }
    if(res.data.hidden1=='1'){
        $('.hide-normal-wifi-checked').attr('checked',true);
    }else {
        $('.hide-normal-wifi-checked').removeAttr('checked');
    }
    if(normalpwdstatus==0) {
        $('.normal-wifi-pwd').val('********');
    }
    $('.normal-wifi-band-width').val(res.data.bandwith1);
    $('.normal-wifi-network-mode').val(res.data.wifimode1);
    $('.normal-wifi-wireless-channel').val(res.data.channel1);
    speedwifiupdate(res);
    if(res.data.disabled2=='0'){
        $('.speed-switch-input').attr('checked',true);
        $('.speed-settings .settings-form input').removeAttr('disabled');
    }else {
        $('.speed-switch-input').removeAttr('checked');
        $('.speed-settings .settings-form input').attr('disabled', 'disabled');
    }
    if(res.data.hidden2=='1'){
        $('.hide-speed-wifi-checked').attr('checked',true);
    }else {
        $('.hide-speed-wifi-checked').removeAttr('checked');
    }
    if(speedpwdstatus==0){
        $('.speed-wifi-pwd').val('********');
    }
    $('.speed-wifi-band-width').val(res.data.bandwith2);
    $('.speed-wifi-network-mode').val(res.data.wifimode2);
    $('.speed-wifi-wireless-channel').val(res.data.channel2);
}
function normalwifiupdate(res) {
    $('.normal-settings .normal-ssid').val(res.data.ssid1);
    $('.normal-encrypt-mode').val(res.data.encryption1);
    // var str1=data.key1;
    // var b = new Base();
    // $('.normal-wifi-pwd').val(b.decode(str1));
}
function speedwifiupdate(res) {
    $('.speed-settings .speed-ssid').val(res.data.ssid2);
    $('.speed-encrypt-mode').val(res.data.encryption2);
    // var str2=data.key2;
    // var b = new Base();
    // $('.speed-wifi-pwd').val(b.decode(str2));
}
$('.logout').click(function () {
    window.localStorage.removeItem('qtec_router_token');
    location.href='./login.html?redirect='+location.pathname+'?t='+Math.random()+location.hash;
})
$('input').on('input',function () {
    var tip=$(this).parent().parent().find('.form-tip').children();
    if(tip.html()){
        tip.html('');
    }
})

var normalpwdstatus=0,speedpwdstatus=0;
$('.normal-wifi-pwd').on('focus',function () {
    if(normalpwdstatus==0){
        $(this).val('');
    }
})
$('.normal-wifi-pwd').on('blur',function () {
    if($(this).val().length==0){
        $(this).val('********');
        normalpwdstatus=0;
    }else {
        normalpwdstatus=1;
    }
})
$('.speed-wifi-pwd').on('focus',function () {
    if(speedpwdstatus==0){
        $(this).val('');
    }
})
$('.speed-wifi-pwd').on('blur',function () {
    if($(this).val().length==0){
        $(this).val('********');
        speedpwdstatus=0;
    }else {
        speedpwdstatus=1;
    }
})


$('.wifi-save-btn').click(function () {
    var ssid1=$('.normal-ssid').val();
    var pwd1=$('.normal-wifi-pwd').val();
    if($('.normal-switch-input').is(':checked')){
        var disabled1='0'
        if(ssid1.length == 0)
        {
            errorShow('.normal-ssid',"2.4Gwifi名称不能为空");
            scrolltotop()
            return;
        }
        // var ssid_filter= (/^[a-zA-Z0-9]{1}([a-zA-Z0-9]|[_-]){5,17}$/).test(ssid1);
        // if(countnums(ssid1)>32){
        if(!check_reg.ssid_reg.test(ssid1)){
            errorShow('.normal-ssid','请输入1-32个位2.4Gwifi名称');
            scrolltotop()
            return;
        }
        if(normalpwdstatus==1)
        {
            // errorShow('.normal-wifi-pwd',"请输入8-32位2.4Gwifi密码");
            // scrolltotop()
            // return;
            // var pwd_filter= (/(?=.*\d)(?=.*[a-zA-Z])(?=.*[^a-zA-Z0-9]).{8,18}/).test(pwd1);
            if(!check_reg.wifi_pwd_reg.test(pwd1)){
                errorShow('.normal-wifi-pwd','请输入8-63位2.4Gwifi密码');
                scrolltotop()
                return;
            }
            if(cn_test(pwd1)==-1){
                errorShow('.normal-wifi-pwd',"2.4Gwifi密码不支持中文字符");
                scrolltotop()
                return
            }
        }
    }else {
        var disabled1='1'
    }
    if($('.hide-normal-wifi-checked').is(':checked')){
        var hidden1='1'
    }else {
        var hidden1='0'
    }
    var encryption1=$('.normal-encrypt-mode').val().toString();
    if(encryption1==''){
        errorShow('.normal-encrypt-mode',"2.4Gwifi加密方式未选择");
        scrolltotop()
        return;
    }
    var bandwith1=$('.normal-wifi-band-width').val().toString();
    if(bandwith1==''){
        errorShow('.normal-wifi-band-width',"2.4Gwifi带宽频段未选择");
        scrolltotop()
        return;
    }
    var wifimode1=$('.normal-wifi-network-mode').val().toString();
    if(wifimode1==''){
        errorShow('.normal-wifi-network-mode',"2.4Gwifi无线模式未选择");
        scrolltotop()
        return;
    }
    var channel1=$('.normal-wifi-wireless-channel').val().toString();
    if(channel1==''){
        errorShow('.normal-wifi-wireless-channel',"2.4Gwifi无线信道未选择");
        scrolltotop()
        return;
    }
    var ssid2=$('.speed-ssid').val();
    var pwd2=$('.speed-wifi-pwd').val();
    if($('.speed-switch-input').is(':checked')){
        var disabled2='0'
        if(ssid2.length == 0 )
        {
            errorShow('.speed-ssid',"5Gwifi名称不能为空");
            return;
        }
        // var ssid_filter= (/^[a-zA-Z0-9]{1}([a-zA-Z0-9]|[_-]){5,17}$/).test(ssid2);
        // if(countnums(ssid2)>32){
        if(!check_reg.ssid_reg.test(ssid2)){
            errorShow('.speed-wifi-pwd','请输入1-32位5Gwifi名称');
            return;
        }
        if(speedpwdstatus==1)
        {
            // errorShow('.speed-wifi-pwd',"请输入8-63位5Gwifi密码");
            // return;
            if(cn_test(pwd2)==-1){
                errorShow('.speed-wifi-pwd',"5Gwifi密码不支持中文字符");
                return
            }
            // var pwd_filter= (/(?=.*\d)(?=.*[a-zA-Z])(?=.*[^a-zA-Z0-9]).{8,18}/).test(pwd2);
            if(!check_reg.wifi_pwd_reg.test(pwd2)){
                errorShow('.speed-wifi-pwd','请输入8-63位5Gwifi密码');
                return;
            }
        }
    }else {
        var disabled2='1'
    }
    if($('.hide-speed-wifi-checked').is(':checked')){
        var hidden2='1'
    }else {
        var hidden2='0'
    }
    var encryption2=$('.speed-encrypt-mode').val().toString();
    if(encryption2==''){
        errorShow('.speed-encrypt-mode',"5Gwifi加密方式未选择");
        return;
    }
    var bandwith2=$('.speed-wifi-band-width').val().toString();
    if(bandwith2==''){
        errorShow('.speed-wifi-band-width',"5Gwifi带宽频段未选择");
        return;
    }
    var wifimode2=$('.speed-wifi-network-mode').val().toString();
    if(wifimode2==''){
        errorShow('.speed-wifi-network-mode',"5Gwifi无线模式未选择");
        return;
    }
    var channel2=$('.speed-wifi-wireless-channel').val().toString();
    if(channel2==''){
        errorShow('.speed-wifi-wireless-channel',"5Gwifi无线信道未选择");
        return;
    }
    resultTipshow(-1,-1);
    var self=this;
    var normalPwdencrypted ='';
    var speedPwdencrypted ='';
    if(normalpwdstatus==1 || speedpwdstatus==1) {
        var data = {
            "requestUrl": "getkeycfg",
            "method": "get"
        };
        $.ajax({
            type: 'POST',
            url: '/cgi-bin/web.cgi/',
            data: JSON.stringify(data),
            dataType: 'json',
            success: function (res) {
                if (res.errorcode == 0) {
                    var encrypt = new JSEncrypt();
                    var publickey = res.data.rand_key;
                    encrypt.setPublicKey(publickey);
                    if (normalpwdstatus==1) {
                        normalPwdencrypted = encrypt.encrypt(pwd1);
                    }
                    if (speedpwdstatus==1) {
                        speedPwdencrypted = encrypt.encrypt(pwd2);
                    }
                    var data={
                        "data": {
                            "disabled1":disabled1,
                            "ssid1":ssid1,
                            "key1":normalPwdencrypted,
                            "hidden1":hidden1,
                            "encryption1":encryption1,
                            "bandwith1":bandwith1,
                            "wifimode1":wifimode1,
                            "channel1":channel1,
                            "disabled2":disabled2,
                            "ssid2":ssid2,
                            "key2":speedPwdencrypted,
                            "hidden2":hidden2,
                            "encryption2":encryption2,
                            "bandwith2":bandwith2,
                            "wifimode2":wifimode2,
                            "channel2":channel2,
                        },
                        "requestUrl": "set_wireless_cfg",
                        "method": "put"
                    }
                    submit(data, self);
                } else {
                    err.error_msg(res);
                }
            }
        })
    }else{
        var data={
            "data": {
                "disabled1":disabled1,
                "ssid1":ssid1,
                "key1":normalPwdencrypted,
                "hidden1":hidden1,
                "encryption1":encryption1,
                "bandwith1":bandwith1,
                "wifimode1":wifimode1,
                "channel1":channel1,
                "disabled2":disabled2,
                "ssid2":ssid2,
                "key2":speedPwdencrypted,
                "hidden2":hidden2,
                "encryption2":encryption2,
                "bandwith2":bandwith2,
                "wifimode2":wifimode2,
                "channel2":channel2,
            },
            "requestUrl": "set_wireless_cfg",
            "method": "put"
        }
        submit(data);
    }
})
function submit(data) {
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        dataType:'json',
        success:function (res) {
            if(res.errorcode==0)
            {
                // if(data.disabled1=='1'){
                //     resultTipshow(1,'保存成功，wifi将会重启，不支持5Gwifi的设备将无法连接');
                // }
                // if(data.disabled2=='1'){
                //     resultTipshow(1,'保存成功，wifi将会重启');
                // }
                if(data.disabled1=='1'&& data.disabled2=='1'){
                    resultTipshow(1,'保存成功，wifi已关闭');
                }else {
                    resultTipshow(1,'保存成功，wifi将会重启');
                }
                wifiInfo(2);
            }else{
                err.error_msg(res);
            }
        },
        error:function () {
            resultTipshow(0,'请求超时，请检查网络连接状态');
        }
    })
}
$('.connect-ways-icon').parent().click(function () {
    getconnectways();
})
var connectinfo;
function getconnectways() {
    var data={
        "requestUrl": "handle_wan_cfg",
        "method": "get"
    };
    postAjax(data,connectwaysfill)
}
function connectwaysfill(res) {
    if(res.errorcode==0){
        switch (res.data.connectiontype) {
            case 'pppoe':
                $('.connect-ways-choose').val(0);
                connectwaysShow(0)
                break;
            case 'dhcp':
                $('.connect-ways-choose').val(1);
                connectwaysShow(1)
                break;
            case 'static':
                $('.connect-ways-choose').val(2);
                connectwaysShow(2);
                break;
            default:
                break;
        }
        connectinfoShow(res);
    }else {
        err.error_msg(res);
    }
}
function connectinfoShow(res) {
    $('.account').val(res.data.username);
    var str = res.data.password;
   if(str){
       var b = new Base();
       $('.pwd').val(b.decode(str));
   }
    $('.ip').val(res.data.ipaddr);
    $('.mask').val(res.data.netmask);
    $('.gateway').val(res.data.gateway);
    var dns=res.data.dns;
    if(dns){
        var i=dns.split(' ')
        if(i.length>1){
            $('.preferreddns').val(i[0]);
            $('.sparedns').val(i[1]);
        }else{
            $('.preferreddns').val(dns);
        }
    }
}
// $('.account').on('input',function () {
//     $(this).val($(this).val().replace(/\s/g,''))
// })
$('.pppoe-save').click(function () {
    var account=$('.account').val()
    var pass=$('.pwd').val();
    var check_result=pppoe_check(account,pass,'.account','.pppoe-pwd');
    if(check_result==-1){
        return;
    }
    resultTipshow(-1,-1);
    var data={
        "requestUrl": "getkeycfg",
        "method": "get"
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        dataType:'json',
        success:function (res) {
            if(res.errorcode==0)
            {
                var encrypt = new JSEncrypt();
                var publickey=res.data.rand_key;
                encrypt.setPublicKey(publickey);
                var encrypted = encrypt.encrypt(pass);
                submit(encrypted);
            }else{
                err.error_msg(res);
            }
        }
    })
    function submit(encrypted,self) {
        var data={"data":{"connectiontype": "pppoe",
            "username": $('.account').val(),
            "password": encrypted
        },
            "requestUrl": "handle_wan_cfg",
            "method": "put"}
       postAjax(data,pppoeset)
    }
})
function pppoeset(res) {
    if(res.errorcode==0)
    {
        resultTipshow(1,'修改成功!');
        getconnectways();
    }else{
        err.error_msg(res);
    }
}
$('.dhcp-save').click(function () {
    resultTipshow(-1,-1);
    var data = {
        "data": {"connectiontype": "dhcp"},
        "requestUrl": "handle_wan_cfg",
        "method": "put"
    }
    postAjax(data,dhcpset)
})
function dhcpset(res) {
    if(res.errorcode==0)
    {
        getconnectways();
        resultTipshow(1,'保存成功');
    }else{
        err.error_msg(res);
    }
}
$('.static-save').click(function () {
    var ip=$('.ip').val();
    var mask=$('.mask').val();
    var gateway=$('.gateway').val();
    var preferreddns=$('.preferreddns').val()
    var sparedns=$('.sparedns').val();
    var check_info={
        "ip":ip,
        "mask":mask,
        "gateway":gateway,
        "preferreddns":preferreddns,
        "sparedns":sparedns
    }
    var tip_var={
        "ip":".ip",
        "mask":".mask",
        "gateway":".gateway",
        "preferreddns":".preferreddns",
        "sparedns":".sparedns"
    }
    var check_result=static_check(check_info,tip_var);
    if(check_result==-1){
        return;
    }
    resultTipshow(-1,-1);
    var data = {
        "data": {
            "connectiontype": "static",
            "ipaddr": ip,
            "netmask": mask,
            "gateway": gateway,
            "dns":check_result
        },
        "requestUrl": "handle_wan_cfg",
        "method": "put"
    }
   postAjax(data,staticset)
})
function staticset(res) {
    if(res.errorcode==0)
    {
        resultTipshow(1,'修改成功');
        getconnectways();
    }else{
        err.error_msg(res);
    }
}
$('.connect-ways-choose').on('change',function () {
    connectwaysShow($(this).val());
    var contype;
        var data={
            "requestUrl": "handle_wan_cfg",
            "method": "get"
        };
   postAjax(data,connectwaychange)
    if($('.error-info').html().length>0){
        errorhide();
    }
})
function connectwaychange(res) {
    if(res.errorcode==0)
    {
        connectinfoShow(res);
    }
}
function resetconnectinfo(contype) {
    switch (contype) {
        case 0:
            $('.connect-ways-choose').val(0);
            $('.static-form input').val('');
            break;
        case 1:
            $('.static-form input').val('');
            $('.pppoe-form input').val('');
            break;
        case 2:
            $('.connect-ways-choose').val(2);
            $('.pppoe-form input').val('');
            break;
        default:
            break;
    }
}
function connectwaysShow(e) {
    if(!($(e).hasClass('active'))){
        $('.form-tabs>div').hide();
        $('.form-tabs>div').eq(e).show();
    }
}
// $('.settings-header>span').click(function () {
//     if($(this).hasClass('no-show')){
//         $('.speed-settings').removeClass('hide');
//         $('.normal-settings').toggleClass('hide');
//     }
//     $('.settings-header>span').toggleClass('no-show');
// })


$('.pwd-switch').click(function () {
    $(this).toggleClass('hidePassword');
    $(this).toggleClass('showPassword');
    if($(this).prev().attr('type')=='password'){
        $(this).prev().attr('type','text')
    }else {
        $(this).prev().attr('type','password')
    }
})

function illegalpwd(m) {
    var cmp = '\\\'"<>';
    var buf = m;
    for (var h = 0; h < buf.length; h++) {
        var tst = buf.substring(h, h + 1);
        if (cmp.indexOf(tst) >= 0) {
            var ss = '密码不能包含' + cmp;
            return ss;
        }
        if (tst.charCodeAt(0) < 0 || tst.charCodeAt(0) > 255) {
            var ss = '密码不支持中文字符';
            return ss;
        }
    }
}

$('.manage-pwd-modify-save-btn').click(function () {
    var oldpwd=$('.manage-old-pwd').val();
    var newpwd=$('.manage-new-pwd').val();
    var confirmpwd=$('.manage-conform-pwd').val();
    if(oldpwd.length==0){
        errorShow('.old-pwd','旧密码不能为空');
        return;
    }
    if(cn_test(oldpwd)==-1){
        errorShow('.old-pwd',"旧密码不支持中文字符");
        return
    }
    if(illegalpwd(oldpwd)){
        errorShow('.old-pwd',illegalpwd(oldpwd));
        return
    }
    if(!check_reg.manage_pwd_reg.test(oldpwd)){
        errorShow('.old-pwd','请输入4-63位旧密码');
        return;
    }
    if(newpwd.length==0){
        errorShow('.new-pwd','新密码不能为空');
        return;
    }
       if(cn_test(newpwd)==-1){
        errorShow('.new-pwd',"新密码不支持中文字符");
        return
    }
    if(illegalpwd(newpwd)){
        errorShow('.new-pwd',illegalpwd(newpwd));
        return
    }
    if(!(check_reg.manage_pwd_reg.test(newpwd))){
        errorShow('.new-pwd','请输入4-63位新密码');
        return;
    }
    if(newpwd===oldpwd){
        errorShow('.new-pwd','新密码和旧密码不能相同');
        return;
    }
    if(confirmpwd.length==0){
        errorShow('.confirm-pwd','确认密码不能为空');
        return;
    }
    if(cn_test(confirmpwd)==-1){
        errorShow('.confirm-pwd',"确认密码不支持中文字符");
        return
    }
    if(illegalpwd(confirmpwd)){
        errorShow('.confirm-pwd',illegalpwd(confirmpwd));
        return
    }
    if(!(check_reg.manage_pwd_reg.test(confirmpwd))){
        errorShow('.confirm-pwd','请输入4-63位确认密码');
        return;
    }
    if(!(newpwd===confirmpwd)){
        errorShow('.confirm-pwd','新密码和确认密码输入不同');
        return;
    }
    resultTipshow(-1,-1);
    var data={
        "requestUrl": "getkeycfg",
        "method": "get"
    };
    var oldpwd=$('.manage-old-pwd').val();
    var newpwd=$('.manage-new-pwd').val();
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode == 0) {
                var encrypt = new JSEncrypt();
                var publickey = res.data.rand_key;
                encrypt.setPublicKey(publickey);
                var oldpwdencrypted = encrypt.encrypt(oldpwd);
                var newpwdencrypted = encrypt.encrypt(newpwd);
                var data={"data":{
                    "username":  "admin",
                    "oldpassword":oldpwdencrypted,
                    "password":newpwdencrypted
                },
                    "requestUrl":"reset_guipassword_cfg",
                    "method":"put"
                }
                postAjax(data,managechange);
            }else {
                err.error_msg(res);
            }
        }
    })
})
function managechange(res) {
    if (res.errorcode == 0) {
        window.localStorage.removeItem('qtec_router_token');
        resultTipshow(1,'管理密码修改成功,3秒后跳转到登录页面');
        var t=4;
        for(var i=0;i<4;i++){
            setTimeout(function () {
                if(t>1){
                    t--;
                    $('.result-tip').html("管理密码修改成功,"+t+"秒后跳转到登录页面");
                }else {
                    $('.popup-shadow').addClass('hide');
                    $('.result-content').fadeOut();
                    location.href='./login.html?t='+Math.random()
                }
            },i*1000)
        }
    }else{
        err.error_msg(res);
    }

}
$('.regular-reboot-icon').parent().click(function () {
    gettask();
})
var timertask;
function gettask() {
    var data={
        "data": {
            "tasktype":2
        },
        "requestUrl": "get_timertask_cfg",
        "method": "get"
    };
    postAjax(data,timetaskfill)
}
function timetaskfill(res) {
    if(res.errorcode==0){
        timertask=res;
        if(res.data.enable==1){
            $('.reboot-switch-input').attr('checked',true);
            $('.regular-reboot-settings').fadeIn();
        }else {
            $('.reboot-switch-input').attr('checked',false);
            $('.regular-reboot-settings').fadeOut();
        }
        timetaskShow(res);
    }else {
        err.error_msg(res);
    }
}
function timetaskShow(res) {
    if(res.data.hour<10)
    {
        var h='0'+res.data.hour;
    }else {
        var h=res.data.hour;
    }
    if(res.data.minute<10)
    {
        var m='0'+res.data.minute;
    }else {
        var m=res.data.minute;
    }
    $('.hour').val(h);
    $('.minute').val(m);
    $('.cycle-list input').removeAttr('checked');
    if(res.data.day.length>0){
        var t=res.data.day.split(',');
        for (var i = 0; i < t.length; ) {
            var e=Number(t[i]);
            checked(e-1);
            i++;
        }
    }
}
var inputVal='';
$('.reboot-time input').focus(function () {
    inputVal= $(this).val();
    $(this).val('');
})
$('.reboot-time input').blur(function () {
    if($(this).val().length==0){
        $(this).val(inputVal);
    }else {
        if($(this).val().length==1){
            var n=$(this).val()
            $(this).val(0+n);
        }
        // $(this).val($(this).val().replace(/\D/g,''))
    }
})
$('.reboot-time input').on('input',function () {
    $(this).val($(this).val().replace(/\D/g,''))
    if($(this).hasClass('hour')){
        if($(this).val()>23){
            $(this).val(23)
        }
    }
    if($(this).hasClass('minute')){
        if($(this).val()>59){
            $(this).val(59)
        }
    }
})
$('.regular-start-save-btn').click(function () {
    if($('.reboot-switch-input').is(':checked')){
        if(!$('.hour').val() || !$('.minute').val()){
            errorShow('.reboot-time .minute+label','重启时间请输入完整')
            return
        }
        if($('.reboot-time input').val().match(/\D/g,'')){
            errorShow('.reboot-time .minute+label','重启时间不支持非数字字符')
            return
        }
        if($('.reboot-time .hour').val()>23){
            errorShow('.reboot-time .minute+label','重启小时区间为0-23')
            return
        }
        if($('.reboot-time .minute').val()>59){
            errorShow('.reboot-time .minute+label','重启分钟区间为0-59')
            return
        }
        var weeks=$('.cycle-list input');
        for(var i=0,n=0;i<weeks.length;){
            if (weeks.eq(i).is(':checked')){
                if(i<6){
                    i++;
                    n=n+','+i;
                }else{
                    i++;
                    n=0+','+n;
                }
            }else {
                i++;
            }
        }
        if(n==0){
            errorShow('.sunday','请选择重复周期')
            return
        }else {
            n=n.substring(2,n.length);
            resultTipshow(-1,-1);
        }
        var h=Number($('.hour').val())
        var m=Number($('.minute').val())
        var data={
            "data": {
                "enable":1,
                "tasktype":2,
                "minute":m,
                "hour":h,
                "day":n
            },
            "requestUrl": "set_timertask_cfg",
            "method": "put"
        }
    }else {
        resultTipshow(-1,-1);
        var data={
            "data": {
                "enable":0,
                "tasktype":2,
                "minute":timertask.data.minute,
                "hour":timertask.data.hour,
                "day":timertask.data.day
            },
            "requestUrl": "set_timertask_cfg",
            "method": "put"
        }
    }
    postAjax(data,timetaskset)
})
function timetaskset(res) {
    if (res.errorcode == 0) {
        gettask();
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}
$('.lan-setting-icon').parent().click(function () {
    getlanInfo();
})
var lan_ip;
function getlanInfo() {
    var data={
        "requestUrl": "get_lan_cfg",
        "method": "get"
    }
    postAjax(data,laninfofill)
}
function laninfofill(res) {
    if (res.errorcode == 0) {
        lan_ip=res.data.ipaddress;
        var lan_mask=res.data.netmask
        $('.lan-ip').val(lan_ip);
        $('.lan-mask').val(lan_mask);
        if(res.data.dhcp_enable==1){
            $('.lan-switch-input').attr('checked',true);
            $('.pool-start').removeAttr('disabled');
            $('.pool-end').removeAttr('disabled');
        }else{
            $('.lan-switch-input').removeAttr('checked');
            $('.pool-start').attr('disabled','disabled');
            $('.pool-end').attr('disabled','disabled');
        }
        lanpoolstart=res.data.poolstart;
        lanpoolend=res.data.poollimit
        $('.pool-start').val(lanpoolstart);
        $('.pool-end').val(lanpoolend);
        dhcp_pool_calculator.formatDhcpPool(lan_ip,lan_mask);
    }else {
        err.error_msg(res);
    }
}
$('.dhcp-switch').click(function () {
    if($(this).prev().is(":checked")){
        $('.pool-start').removeAttr('disabled');
        $('.pool-end').removeAttr('disabled');
    }else {
        $('.pool-start').val(lanpoolstart);
        $('.pool-end').val(lanpoolend);
        $('.pool-start').attr('disabled','disabled');
        $('.pool-end').attr('disabled','disabled');
    }
})
// $('.mask').focus(function () {
//     document.onkeydown=function(event){
//         var e = event || window.event || arguments.callee.caller.arguments[0];
//         if(e && e.keyCode==9){
//             $('.mask').val('255.255.255.0');
//             if($('.error-info').html().length>0){
//                 errorhide();
//             }
//         }
//     };
// })
// $('.mask').blur(function () {
//     $('.mask').val('255.255.255.0');
//     if($('.error-info').html().length>0){
//         errorhide();
//     }
// })

$(".lan-ip").unbind("keyup paste").bind("keyup paste",function(){
    var _val = $(this).val();
    var obj = dhcp_pool_calculator.calculateIPCIDR(_val);
    if(!!obj){
        $(".pool-start").val(obj["dhcpStart"]);
        $(".pool-end").val(obj["dhcpEnd"]);
    }
});
$(".lan-mask").unbind("keyup paste").bind("keyup paste",function(){
    var _val = $(this).val();
    var obj = dhcp_pool_calculator.calculateSubnet(_val);
    if(!!obj){
        $(".pool-start").val(obj["dhcpStart"]);
        $(".pool-end").val(obj["dhcpEnd"]);
    }
});

//计算地址池
var dhcp_pool_calculator = {
    nAddr : new Array(0,0,0,0),
    nMask : new Array(0,0,0,0),
    ndhcpStart : new Array(0,0,0,0),
    ndhcpEnd : new Array(0,0,0,0),
    dec2octet:function(d){//数字还原成IP
        var zeros = "00000000000000000000000000000000";
        var b = d.toString(2);
        var b = zeros.substring(0,32-b.length) + b;
        var a = new Array(
            parseInt(b.substring(0,8),2)
            , (d & 16711680)/65536
            , (d & 65280)/256
            , (d & 255)
        );
        return a;
    },
    octet2dec:function(a){//IP变成数字
        var d = 0;
        d = d + parseInt(a[0]) * 16777216 ;
        d = d + a[1] * 65536;
        d = d + a[2] * 256;
        d = d + a[3];
        return d;
    },
    calculateIPCIDR:function(ip){
        this.nAddr = ip.split('.');
        for(var i in this.nAddr)
            this.nAddr[i] = parseInt(this.nAddr[i]);
        return this.getDhcpRangeStr();
    },
    getDhcpRangeStr: function(){
        var obj = {};
        var wc = this.wildcardMask(this.nMask);

        var aStart = this.startingIP(this.nAddr,this.nMask);
        var aEnd = this.endingIP(this.nAddr,wc);

        var ip1 = this.octet2dec(this.nAddr);
        var ip2 = this.octet2dec(aStart);
        var ip3 = this.octet2dec(aEnd);

        //算出范围后再一次修正范围
        //caculateIPNum(ip1,ip2,ip3);

        if(ip1 == ip2){
            aStart[3] = aStart[3]+1;
        }
        else if(ip1 == ip3){
            aEnd[3] = aEnd[3]-1;
        }
        this.ndhcpStart = aStart;
        this.ndhcpEnd = aEnd;
        obj.dhcpStart = aStart[0] + "." + aStart[1] + "." + aStart[2] + "." + aStart[3];
        obj.dhcpEnd = aEnd[0] + "." + aEnd[1] + "." + aEnd[2] + "." + aEnd[3];
        return obj;

    },
    formatDhcpPool:function(ip,mask){
        var ip_arr = ip.split(".");
        var mask_arr = mask.split(".");
        for(var j = 0; j < 4; j++){
            this.nAddr[j] = parseInt(ip_arr[j],10);
            this.nMask[j] = parseInt(mask_arr[j],10);
        }
        var wc = this.wildcardMask(this.nMask);
        this.ndhcpStart = this.startingIP(this.nAddr,this.nMask);
        this.ndhcpEnd = this.endingIP(this.nAddr,wc);

        var ip_1 = this.octet2dec(this.nAddr);
        var ip_2 = this.octet2dec(this.ndhcpStart);
        var ip_3 = this.octet2dec(this.ndhcpEnd);

        if(ip_1 == ip_2){
            this.ndhcpStart[3] = this.ndhcpStart[3]+1;
        }
        else if(ip_1 == ip_3){
            this.ndhcpEnd[3] = this.ndhcpEnd[3]-1;
        }
    },
    calculateSubnet:function(mask){
        var a = mask.split('.');
        this.nMask[0] = parseInt(a[0]);
        this.nMask[1] = parseInt(a[1]);
        this.nMask[2] = parseInt(a[2]);
        this.nMask[3] = parseInt(a[3]);
        return this.getDhcpRangeStr();
    },
    startingIP:function(aNet,aMask){
        var a = this.subnetID(aNet,aMask);
        var d = this.octet2dec(a);
        d = d+1;
        return this.dec2octet(d);
    },
    endingIP:function(aNet,aWild){
        var a = new this.broadcast(aNet,aWild);
        var d = this.octet2dec(a);
        d = d-1;
        return this.dec2octet(d);
    },
    subnetID:function(aNet,aMask){
        var a = new Array(0,0,0,0);
        for(var i=0;i<4;i++){
            a[i] = aNet[i] & aMask[i];
        }
        return a;
    },
    broadcast:function(aNet,aWild){
        var a = new Array(0,0,0,0);
        for(var i=0;i<4;i++){
            a[i] = aNet[i] | aWild[i];
        }
        return a;
    },
    wildcardMask:function(aMask){
        var a = new Array(0,0,0,0);
        for(var i=0;i<4;i++){
            a[i] = 255 - aMask[i];
        }
        return a;
    }
};
var lanip;
$('.lan-save-btn').click(function () {
    lanip=$('.lan-ip').val()
    if(lanip.length==0){
        errorShow('.lan-ip','ip地址不能为空');
        return;
    }
    // var illegal=(/^\d\.$/)
    // if(illegal.test(lanip)){
    //     errorShow('.lan-ip','请删除非数字和.以外的字符或空格');
    //     return;
    // }
    if(spacecheck(lanip) == -1){
        errorShow('.lan-ip','ip地址不支持空格');
        return;
    }
    var check=(/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/);
    var ip_filter = check.test(lanip);
    if(!ip_filter){
        // errorShow('.lan-ip','请输入正确的ip地址');
        ipcommoncheck('.lan-ip');
        return;
    }
    var lanmask=$('.lan-mask').val()
    if(lanmask.length==0){
        errorShow('.lan-mask','子网掩码地址不能为空');
        return;
    }
    if(spacecheck(lanmask) == -1){
        errorShow('.lan-mask','子网掩码地址不支持空格');
        return;
    }
    var mask_filter = (/^(254|252|248|240|224|192|128|0)\.0\.0\.0$|^(255\.(254|252|248|240|224|192|128|0)\.0\.0)$|^(255\.255\.(254|252|248|240|224|192|128|0)\.0)$|^(255\.255\.255\.(254|252|248|240|224|192|128|0))$/).test(lanmask);
    if(!mask_filter){
        errorShow('.lan-mask','请输入正确的子网掩码地址');
        return;
    }
    if($('.dhcp-switch').prev().is(":checked")){
        var poolstart=$('.pool-start').val();
        var poolend=$('.pool-end').val();
        var laniparr=lanip.split('.');
        var poolstartarr=poolstart.split('.');
        var poolendarr=poolend.split('.');
        if($('.lan-switch-input').is(':checked')){
            if(poolstart.length==0){
                errorShow('.pool-start','地址池开始ip不能为空');
                return;
            }
            if(spacecheck(poolstart) == -1){
                errorShow('.pool-start','地址池开始ip不支持空格');
                return;
            }
            if(poolend.length==0){
                errorShow('.pool-end','地址池结束ip不能为空');
                return;
            }
            if(spacecheck(poolend) == -1){
                errorShow('.pool-end','地址池结束ip不支持空格');
                return;
            }
            if(!check.test(poolstart)){
                // errorShow('.pool-start','地址池开始ip格式错误');
                ipcommoncheck('.pool-start');
                return;
            }
            if(!check.test(poolend)){
                // errorShow('.pool-end','地址池结束ip格式错误');
                ipcommoncheck('.pool-end');
                return;
            }
            for(var i=0;i<3;i++){
                if(poolstartarr[i]>poolendarr[i]){
                    errorShow('.pool-start','地址池开始ip不能大于地址池结束ip');
                    return;
                }
            }
            if(poolstartarr[0]==laniparr[0]&&poolstartarr[1]==laniparr[1]&&poolstartarr[2]==laniparr[2]&&poolstartarr[3]==laniparr[3]){
                errorShow('.pool-start','地址池开始ip不能等于网关ip');
                return;
            }
            if(poolendarr[0]==laniparr[0]&&poolendarr[1]==laniparr[1]&&poolendarr[2]==laniparr[2]&&poolendarr[3]==laniparr[3]){
                errorShow('.pool-end','地址池结束ip不能等于网关ip');
                return;
            }
        }
        dhcp_pool_calculator.formatDhcpPool(lanip,lanmask);
        var obj = dhcp_pool_calculator.calculateIPCIDR(lanip);
        for(var i=0;i<3;i++){
            if(poolstartarr[i]<obj["dhcpStart"].split('.')[i] || poolstartarr[i]>obj["dhcpEnd"].split('.')[i]){
                errorShow('.pool-start','地址池开始ip第'+(i+1)+'段错误');
                return;
            }
            if(poolendarr[i]<obj["dhcpStart"].split('.')[i] || poolendarr[i]>obj["dhcpEnd"].split('.')[i]){
                errorShow('.pool-end','地址池开始ip第'+(i+1)+'段错误');
                return;
            }
        }
    }
    // if(!(laniparr[0]==poolstartarr[0] && laniparr[1]==poolstartarr[1] && laniparr[2]==poolstartarr[2])){
    //     errorShow('.lan-ip','ip地址（前3位）与ip池不在同一网段');
    //     return;
    // }
    // if(!(poolendarr[0]==poolstartarr[0] && poolendarr[1]==poolstartarr[1] && poolendarr[2]==poolstartarr[2])){
    //     errorShow('.pool-start','ip池（前3位）不在同一网段');
    //     return;
    // }
    resultTipshow(-1,-1);
    var data={
        "data":{
            "ipaddress":	lanip,
            "netmask":lanmask,
            "poolStart":	poolstart,
            "poolLimit":	poolend,
            "dhcp_enable": $('.lan-switch-input').is(':checked')==true?1:0
        },
        "requestUrl": "set_lan_cfg",
        "method": "post"
    }
    postAjax(data,lanset,disLoading)
})
function lanset(res) {
    if (res.errorcode == 0) {
        if(lan_ip==lanip){
            resultTipshow(1,'保存成功');
            getlanInfo();
        }else {
            $('.result-icon').removeClass('loading-img');
            resultShowIn('success-icon')
            $('.result-tip').html("有线连接的网络禁用后重启获取新的ip地址，无线连接的网络重新连接wifi后，<a href='http://"+lanip+"/login.html?t='+Math.radom() target='_self'>点击跳转到登录页面</a>");
        }
    }else {
        err.error_msg(res);
    }
}
$('.firmware-update-icon').parent().click(function () {
    checkagain('.check-again');
})
$('.check-again').click(function () {
    checkagain(this);
})
function checkagain(self) {
    $('.firmware-version').hide();
    $('.firmware-update-version').hide();
    $('.newest-version').html('正在获取最新固件信息，请稍候...')
    $(self).attr('disabled','disabled');
    $(self).addClass('disabled');
    $(self).find('.disabled').removeClass('hide')
    checkingAjax(0,self);
}
function checkingAjax(i,self) {
    var data={
        "data": {
        },
        "requestUrl": "get_update_version",
        "method": "get"
    }
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode == 0 && i>=2) {
                $(self).removeAttr('disabled');
                $(self).removeClass('disabled');
                $(self).find('.disabled').addClass('hide')
                var version= res.data.localversionNo
                $('.version').html(version);
                var updateversion=res.data.updateversionNo
                if(res.data.effectivity==1){
                    $('.newest-version').html('检测到新版固件：'+updateversion)
                    $('.firmware-version').fadeIn();
                    $('.firmware-update-version').fadeIn();
                }else {
                    $('.newest-version').html('当前已是最新版本!');
                    $('.firmware-version').fadeOut();
                    $('.firmware-update-version').fadeOut();
                }
            }else {
                if(i>=2){
                    $(self).removeAttr('disabled');
                    $(self).removeClass('disabled');
                    $(self).find('.disabled').addClass('hide')
                    $('.newest-version').html('获取固件信息失败，请检查网络后重试')
                }else {
                    i++;
                    setTimeout(function () {
                        checkingAjax(i,self);
                    },500)
                }
                // if(res.errorcode!='-75'){
                //     err.error_msg(res);
                // }
            }
        },
        error:function () {
            $(self).removeAttr('disabled');
            $(self).removeClass('disabled');
            $(self).find('.disabled').addClass('hide')
            $('.newest-version').html('获取固件信息失败，请检查网络后重试')
        }
    })
}
$('.update-just').click(function () {
    $('.keepconfig').fadeIn();
})
$('.upgrade-cancel').click(function () {
    $('.keepconfig').fadeOut();
})
$('.config-checked input').click(function () {
    if($(this).is(':checked')){
        $('.config-tip').html('已选择保存配置');
    }else {
        $('.config-tip').html('已选择不保存配置');
    }
})
$('.upgrade-confirm').click(function () {
    $('.keepconfig').hide();
    $('.progress-loading').removeClass('hide');
    if($('.config-checked input').is(':checked')){
        var keepconfig=1;
    }else {
        var keepconfig=0;
    }
    var data={
        "data": {
            "keepconfig": keepconfig
        },
        "requestUrl": "upgrade",
        "method": "put"
    }
    postAjax(data,upguadeOnline);
})
function upguadeOnline(res) {
    if (res.errorcode == 0) {
        $('.version-check').hide();
        $('.upgrade-progress').fadeIn();
        upgraderate();
    }else {
        $('.progress-loading').addClass('hide');
        err.error_msg(res);
    }
}
function upgraderate() {
    var data={
        "data": {
        },
        "requestUrl": "get_update_rate",
        "method": "get"
    }
    postAjax(data,upgraderating,upgraderatsuccess);
}
var upgradetry=0;
function upgraderating(res) {
    if (res.errorcode == 0) {
        if(res.data.status=='IMG CHECK FAILED\n'){
            upgradetry++;
            if(upgradetry>10){
                $('.failed-info span').html('固件校验失败，请重试');
                $('.upgrade-progress').hide();
                $('.update-failed').fadeIn();
                return
            }else {
                $('.upgrade-status').html('固件校验失败，正在重试，请稍候... ');
            }
        }else {
            switch (res.data.status){
                case 'IMG UPLOADING':
                    $('.upgrade-status').html('固件下载中');
                    break;
                case'IMG CHECKING':
                    $('.upgrade-status').html('正在校验固件');
                    break;
                case'IMG FLASHING':
                    $('.upgrade-status').html('固件更新完成');
                    break;
                default:
                    break;
            }
        }
        setTimeout(function () {
            upgraderate();
        },500)
    }else {
        $('.progress-loading').addClass('hide');
        err.error_msg(res);
    }
}
function upgraderatsuccess() {
    window.localStorage.removeItem('qtec_router_token');
    var t=121;
    for (var i = 0; i <= 121; i++) {
        window.setTimeout(function () {
            if (t > 1) {
                t--;
                $('.upgrade-status').html('正在重启，预计剩余'+t+'s');
                // $('.upgrade-cutdown').html(t);
            } else {
                location.href='./index.html?t='+Math.random();
            }
        }, i * 1000)
    }
}
$('.failed-confirm').click(function () {
    $('.update-failed').hide();
    $('.version-check').fadeIn();
    checkagain('.check-again');
})
$('.reboot-switch-input').next().click(function () {
    if($('.reboot-switch-input').is(':checked')){
        fadeIn($('.regular-reboot-settings'));
    }else {
        fadeOut($('.regular-reboot-settings'));
        timetaskShow(timertask);
        errorhide()
    }
});
function checked(e) {
    $('.cycle-list input').eq(e).attr('checked', true);
}

$('.all').click(function () {
    $('.cycle-list input').attr('checked', true);
})
$('.notAll').click(function () {
    $('.cycle-list input').attr('checked', false);
})
$('.inverse').click(function () {
    $('.cycle-list input').each(function () {
        $(this).attr('checked',!$(this).is(':checked'))
    });
})
$('.workingday').click(function () {
    for(var i=0;i<$('.cycle-list input').length;){
        if(i<5) {
            $('.cycle-list input').eq(i).attr('checked', true);
        }else{
            $('.cycle-list input').eq(i).attr('checked', false);
        }
        i++;
    }
})
$('.weekend').click(function () {
    for(var i=0;i<$('.cycle-list input').length;){
        if(i<5) {
            $('.cycle-list input').eq(i).attr('checked', false);
        }else{
            $('.cycle-list input').eq(i).attr('checked', true);
        }
        i++;
    }
})
$('.cycle-list span').click(function () {
    $(this).prev().attr('checked', !$(this).prev().is(':checked'));
    errorhide();
})
$('.reboot-btn').click(function () {
    $('.reboot-popup').fadeIn()
})
$('.reboot-cancel').click(function () {
    $('.reboot-popup').fadeOut()
})
$('.cancel').click(function () {
    $(this).parents('.tip-content').parent().addClass('hide');
})
$('.reboot-confirm').click(function () {
    $('.progress-loading').removeClass('hide');
    var data={
        "requestUrl": "reboot",
        "method": "put"
    };
   postAjax(data,rebootsuccess)
})
function rebootsuccess(res) {
    if(res.errorcode==0){
        window.localStorage.removeItem('qtec_router_token');
        var t=81;
        for (var i = 0; i <= 81; i++) {
            $('.reboot-popup').hide();
            $('.reboot-info').hide()
            $('.reboot-loading').fadeIn();
            window.setTimeout(function () {
                if (t > 1) {
                    t--;
                    $('.reboot-cutdown').html(t);
                } else {
                    location.href='./login.html?t='+Math.random();
                }
            }, i * 1000)
        }
    }else {
        $('.progress-loading').addClass('hide');
        err.error_msg(res);
    }
}
$('.restore-btn').click(function () {
    $('.restore-tip').fadeIn();
})
$('.restore-cancel').click(function () {
    $('.restore-tip').fadeOut();
})
$('.restore-confirm').click(function () {
    $('.progress-loading').removeClass('hide');
    var data={
        "requestUrl": "restore",
        "method": "put"
    };
   postAjax(data,restoresuccess)
})
function restoresuccess(res) {
    if(res.errorcode==0){
        window.localStorage.removeItem('qtec_router_token');
        var t=81;
        for (var i = 0; i <= 81; i++) {
            $('.restore-tip').hide();
            $('.restore-info').hide();
            $('.restore-loading').fadeIn();
            window.setTimeout(function () {
                if (t > 1) {
                    t--;
                    $('.restore-cutdown').html(t);
                } else {
                    location.href='http://192.168.1.1';
                }
            }, i * 1000)
        }
    }else {
        $('.progress-loading').addClass('hide');
        err.error_msg(res);
    }
}