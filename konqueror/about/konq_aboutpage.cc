
#include "konq_aboutpage.h"
#include "konq_mainwindow.h"
#include "konq_frame.h"
#include "konq_view.h"

#include <qvbox.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kinstance.h>
#include <khtml_part.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kdebug.h>

#include <assert.h>

extern "C"
{
    void *init_libkonqaboutpage()
    {
        return new KonqAboutPageFactory;
    }
}

KInstance *KonqAboutPageFactory::s_instance = 0;
QString *KonqAboutPageFactory::s_intro_html = 0;
QString *KonqAboutPageFactory::s_specs_html = 0;
QString *KonqAboutPageFactory::s_tips_html = 0;

KonqAboutPageFactory::KonqAboutPageFactory( QObject *parent, const char *name )
    : KParts::Factory( parent, name )
{
    s_instance = new KInstance( "konqaboutpage" );
}

KonqAboutPageFactory::~KonqAboutPageFactory()
{
    delete s_instance;
    s_instance = 0;
    delete s_intro_html;
    s_intro_html = 0;
    delete s_specs_html;
    s_specs_html = 0;
    delete s_tips_html;
    s_tips_html = 0;
}

KParts::Part *KonqAboutPageFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                      QObject *parent, const char *name,
                                                      const char *, const QStringList & )
{
    KonqFrame *frame = dynamic_cast<KonqFrame *>( parentWidget );

    if ( !frame )
        return 0;

    return new KonqAboutPage( frame->childView()->mainWindow(),
                              parentWidget, widgetName, parent, name );
}

QString KonqAboutPageFactory::loadFile( const QString& file )
{
    QString res;
    if ( file.isEmpty() )
	return res;
    
    QFile f( file );
    if ( !f.open( IO_ReadOnly ) )
	return res;

    QByteArray data = f.readAll();
    f.close();

    data.resize( data.size() + 1 );
    data[ data.size() - 1 ] = 0;

    res = QString::fromLatin1( data.data() );
    // otherwise all embedded objects are referenced as about:/...
    QString basehref = QString::fromLatin1("<BASE HREF=\"file:") +
		       file.left( file.findRev( '/' )) +
		       QString::fromLatin1("/\">\n");
    res.prepend( basehref );
    return res;
}

QString KonqAboutPageFactory::intro()
{
    if ( s_intro_html )
        return *s_intro_html;

    QString res = loadFile( locate( "data", "konqueror/about/intro.html" ));
    if ( res.isEmpty() )
	return res;

    res = res.arg( i18n("Conquer your Desktop!") )
          .arg( i18n("Please enter an internet address here.") )
          .arg( i18n( "Introduction" ) )
          .arg( i18n( "Tips" ) )
          .arg( i18n( "Specifications" ) )
          .arg( i18n( "Introduction" ) )
          .arg( i18n( "Welcome to Konqueror." ) )
          .arg( i18n( "With Konqueror you have your filesystem at your command, browsing "
		      "local or networked drives with equal ease. Thanks to the component "
		      "technology used throughout KDE 2, Konqueror is also a full featured, "
		      "easy to use, and comfortable Web Browser, which you can use to explore "
		      "the internet." ) )
          .arg( i18n( "Simply enter the internet address (e.g. " ) )
          .arg( i18n( ") of the webpage you want and press enter. Or choose one of the "
		      "entries in your bookmark-menu. If you want to go back to the previous "
		      "webpage, press the button " ) )
          .arg( i18n( "(\"back\") in the toolbar. To go back to the home-directory of your "
		      "local filesystem press " ) )
          .arg( i18n( "(\"Home\"). For more detailed documentation on Konqueror click " ) )
          .arg( i18n( "here" ) )
          .arg( i18n( "Continue" ) )
//           .arg( i18n( "" ) )
          ;


    s_intro_html = new QString( res );

    return res;
}

QString KonqAboutPageFactory::specs()
{
    if ( s_specs_html )
        return *s_specs_html;

    QString res = loadFile( locate( "data", "konqueror/about/specs.html" ));
    if ( res.isEmpty() )
	return res;

    res = res.arg( i18n("Conquer your Desktop!") )
          .arg( i18n("Please enter an internet address here.") )
          .arg( i18n("Introduction") )
          .arg( i18n("Tips") )
          .arg( i18n("Specifications") )
          .arg( i18n("Specifications") )
          .arg( i18n("Konqueror is designed to embrace and support Internet standards."
		     "The aim is to fully implement the officially sanctioned standards "
		     "from organisations such as the W3 and OASIS, while also adding "
		     "extra support for other common usability features that arise as "
		     "de facto standards across the internet. Along with this support, "
		     "for such functions as favicons, Internet Keywords, and ") )
          .arg( i18n("Konqueror also implements:") )
          .arg( i18n("S P E C I F I C A T I O N S") )
          .arg( i18n("Supported standards") )
          .arg( i18n("Additional requirements*") )
          .arg( i18n("(Level 1, partially Level 2) based ") )
          .arg( i18n("built-in") )
          .arg( i18n("(CSS 1, partially CSS 2)") )
          .arg( i18n("built-in") )
          .arg( i18n("Edition 3 (equals roughly Javascript 1.5)") )
          .arg( i18n("Enable Javascript (globally)") )
          .arg( i18n("here") )
          .arg( i18n("support") )
          .arg( i18n("JDK 1.2.0 (Java 2) compatible VM (") )
          .arg( i18n("Enable Java (globally) ") )
          .arg( i18n("here") )
          .arg( i18n("Netscape Communicator") )
          .arg( i18n("plugins (for viewing ") )
          .arg( i18n("Flash") )
          .arg( i18n("Real") )
          .arg( i18n("Audio, RealVideo etc.)") )
          .arg( i18n("OSF/Motif") )
          .arg( i18n("-compatible library (") )
          .arg( i18n("Open Motif") )
          .arg( i18n("or") )
          .arg( i18n("LessTif") )
          .arg( i18n("Secure Sockets Layer") )
          .arg( i18n("(TLS/SSL v2/3) for secure communications up to 168bit") )
          .arg( i18n("OpenSSL") )
          .arg( i18n("Bidirectional 16bit unicode support") )
          .arg( i18n("built-in") )
          .arg( i18n("Image formats:") )
          .arg( i18n("built-in") )
          .arg( i18n("Transfer protocols:") )
          .arg( i18n("HTTP 1.1 (including gzip/bzip2 compression)") )
          .arg( i18n("FTP") )
          .arg( i18n("built-in") )
          .arg( i18n("Back") )
          .arg( i18n("to the Introduction") )
          //.arg( i18n("") )
          ;

    s_specs_html = new QString( res );

    return res;
}

QString KonqAboutPageFactory::tips()
{
    if ( s_tips_html )
        return *s_tips_html;

    QString res = loadFile( locate( "data", "konqueror/about/tips.html" ));
    if ( res.isEmpty() )
	return res;

    res = res.arg( i18n("Conquer your Desktop!") )
          .arg( i18n("Please enter an internet address here.") )
	  .arg( i18n( "Introduction" ) )
	  .arg( i18n( "Tips" ) )
	  .arg( i18n( "Specifications" ) )
	  .arg( i18n( "Tips" ) )
	  .arg( i18n( "Use Internet-Keywords! By typing \"gg:KDE\" one can search the internet "
		      "using google for the search phrase \"KDE\". There are a lot of "
		      "internet-shortcuts predefined to make searching for software or looking "
		      "up certain words in an encyclopedia a breeze. And you can even "
                      "<A HREF=\"%1\">create your own</A> internet-keywords!" ).arg("exec:/kcmshell ebrowsing") )
	  .arg( i18n( "Use the magnifier toolbar-buttons to increase the fontsize on your webpage." ) )
	  .arg( i18n( "When you want to paste a new address into the URL-bar you might want to "
		      "clear the current entry by pressing the white-crossed black arrow in the "
		      "toolbar." ) )
	  .arg( i18n( "You can also find the \"Fullscreen Mode\" in the Window-Menu. This Feature "
		      "is very useful for \"talk\" sessions." ) )
	  .arg( i18n( "Divide et impera (lat. \"Divide and Konquer\") -- by splitting a window "
		      "into two Parts (e.g. Window -> Split View Left/Right) you can make konqueror "
		      "appear the way you like. You can even Load some example view-profiles "
		      "(e.g. Midnight-Commander), or create your own ones." ) )
	  .arg( i18n( "Use the <A HREF=\"%1\">user-agent</A> feature if the website you're visiting "
                      "asks you to use a different browser "
		      "(and don't forget to send a complaint to the webmaster!)" ).arg("exec:/kcmshell useragent") )
	  .arg( i18n( "The History in your Sidebar makes sure that you will keep track of the "
		      "pages you have visited recently." ) )
	  .arg( i18n( "Use a caching proxy to speed up your internet-connection." ) )
	  .arg( i18n( "Advanced users will appreciate the konsole which you can embed into "
		      "konqueror (Window -> Show Terminal Emulator)." ) )
	  .arg( i18n( "Thanks to DCOP you can have full control over Konqueror using a script." ) )
	  .arg( i18n( "Continue" ) )
	  //	  .arg( i18n( "" ) )
          ;


    s_tips_html = new QString( res );

    return res;
}


KonqAboutPage::KonqAboutPage( KonqMainWindow *, // TODO get rid of this
                              QWidget *parentWidget, const char *widgetName,
                              QObject *parent, const char *name )
    : KHTMLPart( parentWidget, widgetName, parent, name )
{
    //m_mainWindow = mainWindow;
}

KonqAboutPage::~KonqAboutPage()
{
}

bool KonqAboutPage::openURL( const KURL & )
{
    serve( KonqAboutPageFactory::intro() );
    return true;
}

bool KonqAboutPage::openFile()
{
    return true;
}

void KonqAboutPage::saveState( QDataStream &stream )
{
    browserExtension()->KParts::BrowserExtension::saveState( stream );
}

void KonqAboutPage::restoreState( QDataStream &stream )
{
    browserExtension()->KParts::BrowserExtension::restoreState( stream );
}

void KonqAboutPage::serve( const QString& html )
{
    begin( "about:konqueror" );
    write( html );
    end();
}

void KonqAboutPage::urlSelected( const QString &url, int button, int state, const QString &target )
{
    KURL u( url );
    if ( u.protocol() == "exec" )
    {
        QStringList args = QStringList::split( QChar( ' ' ), url.mid( 6 ) );
        QString executable = args[ 0 ];
        args.remove( args.begin() );
        KApplication::kdeinitExec( executable, args );
        return;
    }

    if ( url == QString::fromLatin1("intro.html") )
    {
	serve( KonqAboutPageFactory::intro() );
        return;
    }
    else if ( url == QString::fromLatin1("specs.html") )
    {
	serve( KonqAboutPageFactory::specs() );
        return;
    }
    else if ( url == QString::fromLatin1("tips.html") )
    {
        serve( KonqAboutPageFactory::tips() );
        return;
    }

    KHTMLPart::urlSelected( url, button, state, target );
}

#include "konq_aboutpage.moc"
